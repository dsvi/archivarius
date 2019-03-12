#include "config.h"
#include "globals.h"
#include "exception.h"
#include "property_tree.h"

using namespace std;
namespace fs=filesystem;
using namespace property_tree;

static
void fill_max_storage_time(Config &to, Property &p){
	try {
		auto str_time = p.value_str();
		u64 mult;
		switch (str_time.back()){
		case 'd':
			mult = 24*3600;
			break;
		case 'w':
			mult = 7*24*3600;
			break;
		case 'm':
			mult = 31*24*3600;
			break;
		case 'y':
			mult = 365*24*3600;
			break;
		default:
			throw Exception("'max-storage-time' value must end on 'd', 'w', 'm' or 'y'");
		}
		from_chars(str_time.begin(), str_time.end(), *to.max_storage_time);
		*to.max_storage_time *= mult;
	} catch (...) {
		throw_with_nested(Exception("line {0}: Wrong 'max-storage-time' value")(p.orig_line()));
	}
}

static
void fill_acl(Config &to, Property &p){
	auto val = p.value_str();
	if (val == "on"){
		to.process_acl = true;
		return;
	}
	if (val == "off"){
		to.process_acl = false;
		return;
	}
	throw Exception("Value for 'acl' must be 'on' or 'off'");
}


static
void fill_compression(Config &to, Property &p){
	//auto val = p.value_str();
	to.zstd.emplace();
//	if (val == "on"){
//		to.zstd.emplace();
//		return;
//	}
//	if (val == "off")
//		return;
//	throw Exception("'compression' can only be 'on' or 'off'");
}

static const string conf_fn = "archivarius.conf"s;

std::vector<Config> read_config()
{
	forward_list<fs::path> folders{"/usr/local/etc", "/etc"};
	auto hf = getenv("HOME");
	if (hf)
		folders.push_front(string(hf) + "/.config");
	fs::path cfg_path;
	for (auto &dir : folders){
		cfg_path = dir / conf_fn;
		if (exists(cfg_path))
			break;
		cfg_path = fs::path();
	}
	if (cfg_path.empty()){
		string paths;
		for (auto &dir : folders)
			paths += "\n" + dir.string();
		/* TRANSLATORS: config file {1} was not found at the directories {0}*/
		throw Exception("{1} was not found at: {0}\n")(paths, conf_fn);
	}
	try{
		Property root_pt = from_file(cfg_path);
		unordered_set<std::string_view> names;
		unordered_set<fs::path> arc_paths;
		vector<Config> cfgs;
		Config def_cfg;
		for (auto &pt : root_pt.subs()){
			if (pt.name() == "task"){
				Config cfg(def_cfg);
				cfg.name = pt.value_str();
				if (names.find(cfg.name) != names.end())
					throw Exception(
				      "{1}:{2}:"
				      "'task' named {0} already exist")(cfg.name, pt.orig_name(), pt.orig_line());
				names.insert(cfg.name);
				try {
					for (auto &taskp : pt.subs()){
						if (taskp.name() == "archive"){
							cfg.archive = taskp.value_str();
							if (arc_paths.find(cfg.archive) != arc_paths.end())
								throw Exception("'task' with such 'archive' attribute already exist. {0}")(cfg.archive);
							arc_paths.insert(cfg.archive);
						}
						else if (taskp.name() == "root"){
							cfg.root = taskp.value_str();
						}
						else if (taskp.name() == "include"){
							for (auto &pt : taskp.subs())
								cfg.files_to_archive.push_back(pt.text());
						}
						else if (taskp.name() == "exclude"){
							for (auto &ex : taskp.subs())
								cfg.files_to_ignore.push_back(ex.text());
						}
						else if (taskp.name() == "max-storage-time"){
							fill_max_storage_time(cfg, taskp);
						}
						else if (taskp.name() == "acl"){
							fill_acl(cfg, taskp);
						}
						else if (taskp.name() == "compression"){
							fill_compression(cfg, taskp);
						}
						else if (taskp.name() == "password"){
							auto pwd = taskp.value_str();
							if (pwd.empty())
								throw Exception("line {0}: 'password' can not be empty")(taskp.orig_line());
							cfg.enc.emplace().password = move(pwd);
						}
						else
							throw Exception("line {0}: unknown parameter {1}")(taskp.orig_line(), taskp.name());
					}
					//TODO: check if required stuff is set
					// note that "root" is not one of them, either it or "include" must be set
					cfgs.push_back(move(cfg));
				} catch (...) {
					throw_with_nested(Exception("In task {0}:")(cfg.name));
				}
			}
		}
		return cfgs;
	}
	catch(...){
		throw_with_nested(Exception("Can't read config file {0}")(cfg_path));
	}
}
