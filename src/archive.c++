#include "archive.h"
#include "globals.h"
#include "exception.h"
#include "catalogue.h"
#include "file_content_creator.h"

using namespace std;
namespace fs = std::filesystem;

void Archiver::recursive_add_from_dir(const fs::path &dir_path)
try {
	std::vector<fs::path> dirs;
	for (auto &e : fs::directory_iterator(dir_path)){
		add(e.path());
		if (e.is_directory())
			dirs.push_back(e.path());
	}
	for (auto &dir : dirs)
		recursive_add_from_dir(dir);
}
catch(std::exception &exp){
	string msg = str(boost::format(tr_text("%1%: Can't get directory contents for %2%\n")) % name % dir_path.string());
	msg += message(exp);
	//find_and_replace(msg, "\n", "\n  ");
	warning(move(msg));
}

void Archiver::add(const fs::path &file_path)
{
	try{
		auto rel_path = root.empty() ? file_path : file_path.lexically_relative(root);
		auto [file, is_valid] = make_file(file_path, move(rel_path));
		if (!is_valid)
		  return;
		if (file.type == Filesystem_state::FILE){
			auto sz = fs::file_size(file_path);
			if (sz != 0){
				file.content_ref = prev_->get_ref_if_exist(rel_path, file.mod_time);
				if (!file.content_ref)
					file.content_ref = creator_->add(file_path);
			}
		}
		next_->add(move(file));
	}
	catch(std::exception &exp){
		string msg = str(boost::format(tr_text("%1%: Skipping %2%:\n")) %name % file_path.string());
		msg += message(exp);
		//find_and_replace(msg, "\n", "\n  ");
		warning(move(msg));
	}
}

void Archiver::archive()
{
	try{
		auto fcc = File_content_creator(archive_path);
		creator_ = &fcc;
		auto prev = catalog->latest_fs_state();
		auto next = Filesystem_state(archive_path);
		prev_ = &prev;
		next_ = &next;
		if (files_to_archive.empty()){
			recursive_add_from_dir(root);
		}
		else{
			for (auto &file : files_to_archive){
				auto full_path = root / file;
				if (fs::is_directory(full_path))
					recursive_add_from_dir(full_path);
				else
					add(full_path);
			}
		}
		creator_->finish();
		next_->commit();
		catalog->add_fs_state(next);
	}
	catch(std::exception &e){
		string msg = str(boost::format(tr_text("Error while archiving %1%:\n")) % name);
		msg += message(e);
		warning(move(msg));
	}
}

