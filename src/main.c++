#include <progress_bar.h>
#include "restore.h"
#include "archive.h"
#include "globals.h"
#include "exception.h"
#include "cmd_line_parser.h"
#include "config.h"
#include "test.h"
#include "testing.h"
#include "version.h"

using namespace std;
using namespace coformat;
namespace fs = std::filesystem;
using namespace archi;

//#define TEST

struct Archive_params{
	fs::path archive_path;
	string name;
	string password;
};

Archive_params get_archive_params(Cmd_line &cmd_line, string &cfg_path){
	auto name = cmd_line.param_str_opt("name");
	auto arch = cmd_line.param_str_opt("archive");
	if ((name and arch) or (!name and !arch))
		throw Exception("Either 'name' or 'archive' should be set in command line. But not both.");
	Archive_params ret;
	if (arch){
		ret.archive_path = *arch;
		ret.password = cmd_line.param_str_opt("password").value_or("");
	}
	else {
		auto cfgs = read_config(cfg_path);
		for (auto &c : cfgs){
			if (name and c.name != *name)
				continue;
			ret.name = c.name;
			ret.archive_path = c.archive;
			if (c.enc)
				ret.password = c.enc->password;
			break;
		}
		if (ret.archive_path.empty())
			throw Exception(tr_txt("Task \'{}\' not found in the config file."))(*name);
	}
	return ret;
}


std::string to_human_readable_time(Time time)
{
	return format("{:%Y %B %d %H:%M:%S}", chrono::time_point_cast<chrono::seconds>(to_sys_clock(time)));
}

int run(int argc, const char *argv[]){
	if (argc < 2){
		cprintln(tr_txt(
		  "usage: archivarius <command> [params]\n\n"
		  "command is one of:\n"
		  "	restore    - restore an archive to some path\n"
		  "	archive    - read config file and execute archiving tasks\n"
		  "	             looks for file archivarius.conf in path:\n"
		  "	             ~/.config\n"
		  "	             /usr/local/etc\n"
		  "	             /etc\n"
		  "	             and follows instructions in it\n"
		  "	list       - list versions in an archive\n"
		  "	list-files - list content of a version in archive\n"
		  "	remove     - removes a version from archive\n"
		  "	test       - check checksums in an archive, and report errors if they dont match\n"
		  "	version    - prints version.\n\n"
		  "params are in the form param1=value param2=value2\n"
		  "params can be:\n"
		  "	archive  - path to the archive. normally either this or 'name' should be set\n"
		  "	name     - name of a task in the config file\n"
		  "	id       - id of the version in the archive. get it with 'list' command\n"
		  "	password - password to the archive\n\n"
		  "Acceptable parameters for commads:\n"
		  "	restore:\n"
		  "		archive\n"
		  "		name\n"
		  "		id\n"
		  "		target-dir - where to restore\n"
		  "		prefix - restore only the paths begining with this prefix\n"
		  "		         works on full path names. so prefix a/b/c will restore\n"
		  "		         a/b/c/d but not a/b/cd.\n"
		  "		         in the above example, only the c will be restored, not a/b.\n"
		  "		password\n"
		  "	archive:\n"
		  "		name - if not set, all tasks will be processed\n"
		  "	list:\n"
		  "		name\n"
		  "		archive\n"
		  "		password\n"
		  "	list-files:\n"
		  "		name\n"
		  "		archive\n"
		  "		password\n"
		  "		id\n"
		  "	test:\n"
		  "		archive\n"
		  "		name\n\n"

		  "example:\n"
		  "	archivarius restore archive=/nfs/backup target-dir=. password=\"qwerty asdfg\"\n"
		  "	archivarius restore archive=/nfs/backup prefix=Pictures target-dir=. password=\"qwerty asdfg\"\n"
		  "	archivarius restore name=\"home folder backup\" prefix=Pictures target-dir=.\n"
		  ));
		return 0;
	}
	auto cmd_line = parse_command_line(argc, argv);
	auto cfg_path = cmd_line.param_str_opt("cfg-file").value_or("");
	optional<Progress_bar> progress;
	int return_code = 0;
	auto report_warning = [&](std::string &&h, std::string &&w){
		cprint(stderr, "{fr}{}{fd}", h);
		w.insert(0, "\n");
		find_and_replace(w, "\n", "\n  ");
		cprintln(stderr, "{}", w);
		fflush(stderr);
		return_code = 1;
		if (progress)
			progress->show();
	};
	auto report_progress = [&](uint progress_in_permil){
		if (!progress)
			progress.emplace();
		progress->update(progress_in_permil/10);
	};
	if (cmd_line.command() == "archive"){
		auto name = cmd_line.param_str_opt("name");
		cmd_line.check_unused_arguments();
		auto cfgs = read_config(cfg_path);
		bool task_found = false;
		for (auto &c : cfgs){
			try {
				if (name and c.name != *name)
					continue;
				task_found = true;
				Archive_action arc;
				cprint("╼╾╼╾╼▏{fy}{}{fd}▕╾╼╾╼╾╼╾╼╾╼╾\n",c.name);
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
					arc.max_storage_time = *c.max_storage_time_seconds * Time_accuracy::period::den;
				arc.password = c.enc.has_value() ? c.enc->password : "";
				if (c.zstd){
					arc.zstd.emplace();
					arc.zstd->compression_level = 11;
				}
				arc.warning = move(report_warning);
				arc.process_acls = c.process_acl;
				arc.archive();
			} catch (std::exception &e) {
				cprint(stderr, tr_txt("{fr}Stopped processing the task.{fd}\n"));
				auto msg = message(e);
				find_and_replace(msg, "\n", "\n  ");
				cprint(stderr, msg);
				fflush(stderr);
			}
		}
		if (name and !task_found)
			throw Exception(tr_txt("Task \'{}\' not found in the config file."))(*name);
	}
	else
	if (cmd_line.command() == "list"){
		auto tp = get_archive_params(cmd_line, cfg_path);
		cmd_line.check_unused_arguments();
		Catalogue cat(tp.archive_path, tp.password, false);
		auto times = cat.state_times();
		for (size_t i = times.size(); i-- > 0; ){
			println("{:┄<5}┄{}", i, to_human_readable_time(times[i]));
		}
	}
	else
	if (cmd_line.command() == "list-files"){
		auto tp = get_archive_params(cmd_line, cfg_path);
		uint id = cmd_line.param_uint_opt("id").value_or(0);
		cmd_line.check_unused_arguments();
		Catalogue cat(tp.archive_path, tp.password, false);
		auto st = cat.fs_state(id);
		for (Filesystem_state::File &file: st.files()){
			cprint("{fg}{}{fd}\n", file.path.string());
			switch (file.type){
			case Filesystem_state::File_type::FILE:
				cprint(tr_txt("File\n"));
				if (file.content_ref.has_value())
					cprintln(tr_txt("Stored in: {}"), file.content_ref->fname);
				break;
			case Filesystem_state::File_type::DIR:
				cprintln(tr_txt("Directory"));
				break;
			case Filesystem_state::File_type::SYMLINK:
				cprintln(tr_txt("Symlink to: {}"), file.symlink_target);
				break;
			default:
				ASSERT(0);
			}
			if ( file.mod_time )
				cprintln(tr_txt("Modification time: {}"), to_human_readable_time(file.mod_time.value()));
			print("\n");
		}
	}
	else
	if (cmd_line.command() == "remove"){
		auto tp = get_archive_params(cmd_line, cfg_path);
		uint id = cmd_line.param_uint("id");
		cmd_line.check_unused_arguments();
		Catalogue cat(tp.archive_path, tp.password, false);
		cat.remove_fs_state(cat.fs_state(id));
		cat.commit();
	}
	else
	if (cmd_line.command() == "restore"){
		Restore_action rs;
		auto tp = get_archive_params(cmd_line, cfg_path);
		rs.archive_path = move(tp.archive_path);
		rs.name = move(tp.name);
		rs.password = move(tp.password);
		rs.to = cmd_line.param_str("target-dir");
		rs.from_ndx = cmd_line.param_uint_opt("id").value_or(0);
		if (auto pref = cmd_line.param_str_opt("prefix"); pref){
			auto p = *pref;
			while (!p.empty() and p.front() == '/')
				p.erase(0,1);
			while (!p.empty() and p.back() == '/')
				p.pop_back();
			rs.prefix = p;
		}
		cmd_line.check_unused_arguments();
		rs.warning = move(report_warning);
		rs.progress = report_progress;
		rs.restore();
	}
	else
	if (cmd_line.command() == "test"){
		Test_action ts;
		ts.warning = move(report_warning);
		auto tp = get_archive_params(cmd_line, cfg_path);
		ts.archive_path = move(tp.archive_path);
		ts.name = move(tp.name);
		ts.password = move(tp.password);
		cmd_line.check_unused_arguments();
		ts.progress_status = [](std::string &&status_text){
			println("{}", move(status_text));
		};
		ts.progress = report_progress;
		ts.test();
		cprintln(tr_txt("Test finished."));
	}
	else
	if (cmd_line.command() == "version"){
		cmd_line.check_unused_arguments();
		println("{}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	}
	else{
		cprintln(stderr, tr_txt("{fr}unknown command:{fd} {}"), cmd_line.command());
		return 2;
	}
	return return_code;
}



int main(int argc, const char *argv[]){
	try{
#ifdef TEST
		test();
		return 0;
#else
		return run(argc, argv);
#endif
	}
	catch(std::exception &e){
		cprintln(stderr, "{fr}{}{fd}", message(e));
		return 2;
	}
}
