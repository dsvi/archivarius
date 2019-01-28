#include "globals.h"
#include "archive.h"
#include "restore.h"
#include "cmd_line_parser.h"
#include "config.h"
#include "exception.h"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char *argv[]){
	try{
		init_epoch();
		if (argc < 2){
			cout << tr_txt(
			  "usage:\n"
			  " blah blah blah\n"
			  " blah\n"
			  ) << endl;
			return 0;
		}
		auto cmd_line = parse_command_line(argc, argv);
		if (cmd_line.command() == "archive"){
			auto cfgs = read_config();
			Archiver arc;
			for (auto &c : cfgs){
				try {
					cout << "-=-=-=- " << c.name << " -=-=-=-"<< endl;
					auto report_warning = [](std::string &&w){
						find_and_replace(w, "\n", "\n  ");
						cerr << w << endl;
					};
					Catalogue cat(c.archive);
					// TODO: remove recalculation of content file size???
					arc.min_content_file_size = 10*1024*1024;
					arc.catalog = &cat;
					// -=- GC -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
					auto max_ref = cat.max_ref_count();
					if (max_ref != 0){
						auto fs_state = cat.latest_fs_state();
						struct Old_files{
							fs::path *path;
							string   *content_fn;
							u64      size;
						};
						list<Old_files> old_enough_to_compact;
						for (Filesystem_state::File &file: fs_state.files()){
							if ( file.content_ref.has_value() && file.content_ref.value().ref_count_ == cat.max_ref_count()){
								old_enough_to_compact.emplace_front();
								auto &a = old_enough_to_compact.front();
								a.path = &file.path;
								a.content_fn = &file.content_ref->fname;
								a.size = file.content_ref->space_taken;
							}
						}
						unordered_map<string, u64> content_file_size;
						u64 total_content_size = 0;
						for (auto &f : old_enough_to_compact){
							if (content_file_size.find(*f.content_fn) != content_file_size.end())
								continue;
							auto size = file_size(c.archive / *f.content_fn);
							if (size < arc.min_content_file_size) // intentionally used here before recalculation later
								content_file_size[*f.content_fn] = arc.min_content_file_size;
							else
								content_file_size[*f.content_fn] = size;
							total_content_size += size;
						}
						{
							u64 new_min_content_size = total_content_size / 100;
							if (new_min_content_size > arc.min_content_file_size)
								arc.min_content_file_size = new_min_content_size;
						}
						for (auto it = old_enough_to_compact.begin(); it != old_enough_to_compact.end();){
							auto &size = content_file_size[*it->content_fn];
							size -= it->size;
							ASSERT(size >= 0);
							if (size == 0)
								it = old_enough_to_compact.erase(it);
							else
								++it;
						}
						auto total_waste = ranges::accumulate(content_file_size | ranges::view::values, u64(0));
						if (total_waste > arc.min_content_file_size * 2){
							for (auto &f : old_enough_to_compact)
								arc.to_compact.insert(*f.path);
						}
					}
					// -=- GC -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
					arc.name = c.name;
					arc.archive_path = c.archive;
					arc.files_to_archive = c.files_to_archive;
					arc.files_to_exclude = c.files_to_ignore;
					arc.root = c.root;
					arc.warning = move(report_warning);
					arc.archive();
				} catch (std::exception &e) {
					cerr << tr_txt("Stopped processing the task.") << endl;
					cerr << message(e) << endl;
				}
			}

		}
		else
		if (cmd_line.command() == "list"){
			fs::path archive_path{cmd_line.param_str("archive")};
			Catalogue cat(archive_path);
			auto times = cat.state_times();
			for (size_t i = 0; i < times.size(); i++){
				auto time_t = fs::file_time_type::clock::to_time_t(times[i]);
				auto tm = gmtime(&time_t);
				cout << i << "\t" << put_time(tm, "%Y %B %d %H:%M:%S") << "\n";
			}
		}
		else
		if (cmd_line.command() == "restore"){
			fs::path archive_path{cmd_line.param_str("archive")};
			fs::path restore_path{cmd_line.param_str("target-dir")};
			auto id_opt = cmd_line.param_uint_opt("id");
			Catalogue cat(archive_path);
			auto num_ids = cat.state_times().size();
			if (num_ids == 0){
				cout << tr_txt("Archive is empty.") << endl;
				return 0;
			}
			size_t id = id_opt.value_or(cat.state_times().size() - 1);
			auto report_warning = [](std::string &&w){
				cerr << w << endl;
			};
			Restore_settings rs;
			rs.from=archive_path;
			rs.from_ndx=id;
			rs.to=restore_path;
			rs.warning=move(report_warning);
			restore(move(rs));
		}
		return 0;
	}
	catch(std::exception &e){
		cerr << message(e) << endl;
		return 1;
	}
}
