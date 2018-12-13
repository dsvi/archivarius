#include "config.h"
#include "globals.h"
#include "exception.h"
#include "property_tree.h"

using namespace std;
namespace fs=filesystem;
using namespace property_tree;

void get_max_storage_time(Config &get_to, string_view str_time){
	try {
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
		from_chars(str_time.begin(), str_time.end(), *get_to.max_storage_time);
		*get_to.max_storage_time *= mult;
	} catch (...) {
		throw_with_nested(Exception("Wrong 'max-storage-time' value"));
	}
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
							continue;
						}
						if (taskp.name() == "root"){
							cfg.root = taskp.value_str();
							continue;
						}
						if (taskp.name() == "include"){
							for (auto &pt : taskp.subs())
								cfg.files_to_archive.push_back(pt.text());
							continue;
						}
						if (taskp.name() == "exclude"){
							for (auto &ex : taskp.subs())
								cfg.files_to_ignore.push_back(ex.text());
							continue;
						}
						if (taskp.name() == "max-storage-time"){
							get_max_storage_time(cfg, taskp.value_str());
							continue;
						}
						throw Exception("unknown parameter {0}")(taskp.name());
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
