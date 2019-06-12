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
		for (auto &c : cfgs){
			try {
				Archive_settings arc;
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

				arc.name = c.name;
				arc.archive_path = c.archive;
				arc.root = c.root;
				arc.files_to_archive = c.files_to_archive;
				for (auto &f: c.files_to_ignore)
					arc.files_to_exclude.insert(move(f));
				if (c.min_content_file_size)
					arc.min_content_file_size = c.min_content_file_size;
				else
					arc.min_content_file_size = 2*1024*1024*1024ul;
				if (c.max_storage_time_seconds)
					arc.max_storage_time = *c.max_storage_time_seconds * Time_ticks_in_second;
				arc.password = c.enc.has_value() ? c.enc->password : "";
				if (c.zstd){
					arc.zstd.emplace();
					arc.zstd->compression_level = 11;
				}
				arc.warning = move(report_warning);
				arc.process_acls = c.process_acl;
				archive(move(arc));
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
		Restore_settings rs;
		rs.archive_path = cmd_line.param_str("archive");
		rs.to = cmd_line.param_str("target-dir");
		rs.from_ndx = cmd_line.param_uint_opt("id").value_or(0);
		rs.password = cmd_line.param_str_opt("password").value_or("");
		cmd_line.check_unused_arguments();
		rs.warning = [](std::string &&w){
			cerr << w << endl;
		};
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
