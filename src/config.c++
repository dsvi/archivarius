#include "config.h"
#include "globals.h"
#include "exception.h"

using namespace std;
namespace fs=std::filesystem;
using namespace boost::property_tree;

std::vector<Config> read_config(const std::filesystem::path &dir)
{
	auto cfg_path = dir / "archivarius.conf";
	try{
		if (!exists(cfg_path))
			throw Exception("File doesn't exist");
		ptree root_pt;
		if (exists(cfg_path))
			read_info(cfg_path, root_pt);
		unordered_set<std::string_view> names;
		unordered_set<fs::path> arc_paths;
		vector<Config> cfgs;
		Config def_cfg;
		for (auto &pt : root_pt){
			if (pt.first == "task"){
				Config cfg(def_cfg);
				auto &sub = pt.second;
				cfg.name = sub.data();
				if (cfg.name.empty())
					throw Exception("'task' is required to have a name");
				if (names.find(cfg.name) != names.end())
					throw Exception("'task' named %1% already exist") << cfg.name;
				names.insert(cfg.name);
				try {
					cfg.archive = sub.get<string>("archive");
					if (arc_paths.find(cfg.archive) != arc_paths.end())
						throw Exception("'task' with such 'archive' attribute already exist. %1%") << cfg.archive;
					arc_paths.insert(cfg.archive);
					for (auto &src : sub.get_child("sources"))
						cfg.files_to_archive.push_back(src.first);
					if (cfg.files_to_archive.empty())
						throw Exception("A 'task' is required to have populated 'sources'");
					auto depth = sub.get_optional<string>("depth");
					if (depth){
						try {
							auto str_time = depth.value();
							if (str_time.empty())
								throw tr_text("'depth' value can't be empty");
							ui64 mult;
							switch (str_time.back()){
							case 'd':
								mult = 24*3600;
								break;
							case 'w':
								mult = 7*24*3600;
								break;
							case 'm':
								mult = 30.5*24*3600;
								break;
							case 'y':
								mult = 365*24*3600;
								break;
							default:
								throw Exception("'depth' value must end on 'd', 'w' pr 'y'");
							}
							cfg.max_storage_time = stoull(str_time) * mult;
						} catch (...) {
							throw_with_nested(Exception("Wrong 'depth' value"));
						}
					}
				} catch (...) {
					throw_with_nested(Exception("In task %1%") << cfg.name);
				}
				cfgs.push_back(move(cfg));
			}
		}
		return cfgs;
	}
	catch(...){
		throw_with_nested(Exception("Can't read config file %1%") << cfg_path);
	}
}
