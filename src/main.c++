#include "globals.h"
#include "archive.h"
#include "restore.h"
#include "cmd_line_parser.h"
#include "config.h"
#include "exception.h"
#include "test.h"

using namespace std;
namespace fs = std::filesystem;

//#define TEST

int run(int argc, const char *argv[]){
	if (argc < 2){
		cout << tr_txt(
			"usage: archivarius <cmd> [params]\n\n"
		  "cmd is one of:\n"
			"	restore - restore the archive to some path\n"
		  "	archive - read config file and execute archiving tasks\n"
		  "	          looks for file archivarius.conf in path:\n"
		  "	          ~/.config\n"
		  "	          /usr/local/etc\n"
		  "	          /etc\n"
		  "	          and follows instructions in it\n"
		  "	list    - list versions in archive\n\n"
		  "Acceptable parameters for commads:\n"
		  "	restore:\n"
		  "		archive\n"
		  "		id\n"
		  "		target-dir - where to restore\n"
		  "		password\n"
		  "	archive:\n"
		  "		\n"
		  "	list:\n"
		  "		archive\n"
		  "		password\n\n"
		  "[params] are in form param1=value param2=value\n"
		  "params can be:\n"
		  "	archive  - path to archive\n"
		  "	id       - id of the state in archive\n"
		  "	password - password to archive\n"
			) << endl;
		return 0;
	}
	auto cmd_line = parse_command_line(argc, argv);
	auto cfg_path = cmd_line.param_str_opt("cfg-file");
	if (cmd_line.command() == "archive"){
		cmd_line.check_unused_arguments();
		auto cfgs = read_config(cfg_path.value_or(""));
		Archiver arc;
		for (auto &c : cfgs){
			try {
				fmt::print("╼╾╼╾╼▏");
				print(fg(fmt::terminal_color::yellow), " {} ",c.name);
				fmt::print("▕╾╼╾╼╾╼╾╼╾╼╾\n");
				fflush(stdout);
				auto report_warning = [](std::string &&h, std::string &&w){
					print(stderr, fg(fmt::terminal_color::red), "Warning! {}", h);
					w.insert(0, "\n");
					find_and_replace(w, "\n", "\n  ");
					w += '\n';
					fmt::print(stderr, w);
					fflush(stderr);
				};
				Catalogue cat(c.archive, c.enc ? c.enc->password : "");
				if (c.min_content_file_size)
					arc.min_content_file_size = c.min_content_file_size;
				else
					arc.min_content_file_size = 2*1024*1024*1024ul;
				arc.catalog = &cat;
				// -=- GC -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
				//TODO: move to archive
				if (c.max_storage_time_seconds){
					auto max_ref = cat.max_ref_count();
					if (max_ref != 0){
						auto fs_state = cat.latest_fs_state();
						struct Old_files{
							fs::path *path;
							string_view content_fn;
							u64      size;
						};
						vector<Old_files> old_enough_to_compact;
						for (Filesystem_state::File &file: fs_state.files()){
							if ( file.content_ref.has_value() && file.content_ref.value().ref_count_ == cat.max_ref_count()){
								auto &a = old_enough_to_compact.emplace_back();
								a.path = &file.path;
								a.content_fn = file.content_ref->fname;
								a.size = file.content_ref->space_taken;
							}
						}
						unordered_map<string_view, u64> content_file_sizes;
						for (auto &f : old_enough_to_compact){
							if (content_file_sizes.find(f.content_fn) != content_file_sizes.end())
								continue;
							auto size = file_size(c.archive / f.content_fn);
							content_file_sizes[f.content_fn] = max(size, arc.min_content_file_size);
						}
						for (auto &f :old_enough_to_compact){
							auto &size = content_file_sizes[f.content_fn];
							size -= min(f.size, size); // underflow protection
						}
						// now content_file_sizes contains wasted space for each file
						unordered_set<string_view> content_files_to_compact;
						for (auto &cz: content_file_sizes){
							if (cz.second < arc.min_content_file_size / 16)
								continue;   // too little is wasted
							content_files_to_compact.insert(cz.first);
						}
						u64 total_size = 0;
						for (auto &f : old_enough_to_compact ){
							if (content_files_to_compact.find(f.content_fn) != content_files_to_compact.end()){
								arc.force_to_archive.insert(*f.path);
								total_size += f.size;
							}
						}
						if (total_size < arc.min_content_file_size) //not enough even for one new content file
							arc.force_to_archive.clear();
					}
				}
				arc.name = c.name;
				arc.encryption = c.enc.has_value();
				arc.process_acls = c.process_acl;
				arc.archive_path = c.archive;
				arc.files_to_archive = c.files_to_archive;
				for (auto &f: c.files_to_ignore)
					arc.files_to_exclude.insert(move(f));
				arc.root = c.root;
				if (c.zstd){
					arc.zstd.emplace();
					arc.zstd->compression_level = 11;
				}
				arc.warning = move(report_warning);
				arc.archive();
				if (c.max_storage_time_seconds){
					try{
						auto t = to_posix_time(fs::file_time_type::clock::now()) - *c.max_storage_time_seconds * Time_ticks_in_second;
						vector<Filesystem_state> states_to_remove;
						size_t ndx = 0;
						for (auto state_time : cat.state_times()){
							if (state_time < t)
								states_to_remove.push_back(cat.fs_state(ndx));
							ndx++;
						}
						// leave at least one state
						if (cat.max_ref_count() == states_to_remove.size())
							states_to_remove.pop_back();
						for (auto &fs_state : states_to_remove)
							cat.remove_fs_state(fs_state);
					}
					catch(std::exception &exp){
						arc.warning( tr_txt("Error while remoing old state"), message(exp) );
					}
				}
				cat.commit();
			} catch (std::exception &e) {
				print(stderr, fg(fmt::terminal_color::red), "Stopped processing the task.\n");
				auto msg = message(e);
				find_and_replace(msg, "\n", "\n  ");
				fmt::print(stderr, msg);
				fflush(stderr);
			}
		}

	}
	else
	if (cmd_line.command() == "list"){
		fs::path archive_path{cmd_line.param_str("archive")};
		auto password = cmd_line.param_str_opt("password");
		cmd_line.check_unused_arguments();
		Catalogue cat(archive_path, password.value_or(""));
		auto times = cat.state_times();
		for (size_t i = 0; i < times.size(); i++){
			time_t t = times[i]/ 1e9;
			auto tm = localtime(&t);
			char str[200];
			if (!strftime(str, sizeof(str), "%Y %B %d %H:%M:%S", tm))
				throw Exception("Can't format time string for current locale");
			fmt::print("{:-<5}-{}\n", i, str);
		}
	}
	else
	if (cmd_line.command() == "restore"){
		fs::path archive_path{cmd_line.param_str("archive")};
		fs::path restore_path{cmd_line.param_str("target-dir")};
		auto id_opt = cmd_line.param_uint_opt("id");
		auto password = cmd_line.param_str_opt("password");
		cmd_line.check_unused_arguments();
		Catalogue cat(archive_path, password.value_or(""));
		auto state_times = cat.state_times();
		auto num_ids = state_times.size();
		if (num_ids == 0){
			cout << tr_txt("Archive is empty.") << endl;
			return 0;
		}
		size_t id = id_opt.value_or(state_times.size() - 1);
		auto report_warning = [](std::string &&w){
			cerr << w << endl;
		};
		Restore_settings rs;
		rs.cat = &cat;
		rs.from_ndx = id;
		rs.to = restore_path;
		rs.warning = move(report_warning);
		restore(rs);
	}
	else{
		print(stderr, fg(fmt::terminal_color::red), tr_txt("unknown command {}\n"), cmd_line.command());
		return 1;
	}
	return 0;
}



int main(int argc, const char *argv[]){
	try{
		init_epoch();
#ifdef TEST
		test();
		return 0;
#else
		return run(argc, argv);
#endif
	}
	catch(std::exception &e){
		print(stderr, fg(fmt::terminal_color::red), message(e));
		return 1;
	}
}
