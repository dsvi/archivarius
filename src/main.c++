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
		auto cmd_line = parse_command_line(argc, argv);
		if (cmd_line.command() == "archive"){
			auto cfgs = read_config();
			Archiver arc;
			for (auto &c : cfgs){
				try {
					cout << c.name << endl;
					auto report_warning = [](std::string &&w){
						cerr << w << endl;
					};
					Catalogue cat(c.archive);



					arc.name = c.name;
					arc.archive_path = c.archive;
					arc.files_to_archive = c.files_to_archive;
					arc.root = c.root;
					arc.warning = move(report_warning);
					arc.archive();
				} catch (std::exception &e) {
					cerr << tr_text("Stopped processing the task.") << endl;
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
				cout << tr_text("Archive is empty.") << endl;
				return 0;
			}
			size_t id = id_opt.value_or(cat.state_times().size() - 1);
			auto report_warning = [](std::string &&w){
				cerr << w << endl;
			};
			restore({.from=archive_path, .from_ndx=id, .to=restore_path, .warning=move(report_warning)});
		}
		return 0;
	}
	catch(std::exception &e){
		cerr << message(e) << endl;
		return 1;
	}
}
