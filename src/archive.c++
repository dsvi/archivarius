#include "archive.h"
#include "globals.h"
#include "exception.h"
#include "catalogue.h"
#include "file_content_creator.h"

using namespace std;
namespace fs = std::filesystem;

static
void recursive_scan(
    fs::path &dir,
    Filesystem_state &prev,
    Filesystem_state &next,
    fs::path &root,
    File_content_creator &creator,
    std::function<void(std::string&&)> &warning)
{


	for (auto &e : fs::directory_iterator(dir)){
		try{
			auto path = e.path();
			auto rel_path = path.lexically_relative(root);
			auto [file, is_valid] = make_file(e, rel_path);
			if (!is_valid)
			  continue;
			if (file.type == Filesystem_state::FILE){
				auto sz = e.file_size();
				if (sz != 0){
					file.content_ref = prev.get_ref_if_exist(rel_path, file.mod_time);
					if (!file.content_ref)
						file.content_ref = creator.add(path);
				}
			}
			else if (file.type == Filesystem_state::DIR){
				recursive_scan(path, prev, next, root, creator, warning);
			}
			next.add(move(file));
		}
		catch(std::exception &exp){
			string msg = str(boost::format(tr_text("Skipping file %1%:\n")) % e.path());
			msg += message(exp);
			warning(move(msg));
		}
	}
}

void archive(Archive_settings &cfg)
{
	try{
		Catalogue catalog(cfg.archive);
		auto prev = catalog.latest_fs_state();
		Filesystem_state next(cfg.archive);
		recursive_scan(cfg.from, prev, next, cfg.from, cfg.creator, cfg.warning);
		next.commit();
		catalog.commit();
	}
	catch(std::exception &e){
		string msg = str(boost::format(tr_text("Error while archiving %1%:\n")) % cfg.name);
		msg += message(e);
		cfg.warning(move(msg));
	}
}
