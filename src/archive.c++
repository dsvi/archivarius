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
		auto p = e.path();
		for (auto &file : files_to_exclude){
			if (p == file)
				continue;
		}
		add(p);
		if (e.is_directory())
			dirs.push_back(e.path());
	}
	for (auto &dir : dirs)
		recursive_add_from_dir(dir);
}
catch(std::exception &exp){
	string msg = fmt::format(tr_txt("Can't get directory contents for {0}:\n"), dir_path);
	msg += message(exp);
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
		string msg = fmt::format(tr_txt("Skipping {0}:\n"), file_path);
		msg += message(exp);
		warning(move(msg));
	}
}

void Archiver::archive()
{
	try{
		auto fcc = File_content_creator(archive_path);
		fcc.min_file_size(min_content_file_size);
		creator_ = &fcc;
		auto prev = catalog->latest_fs_state();
		auto next = Filesystem_state(archive_path);
		prev_ = &prev;
		next_ = &next;
		if (!root.empty()){
			for (auto &file : files_to_archive)
				file = root / file;
			for (auto &file : files_to_exclude)
				file = root / file;
		}
		if (files_to_archive.empty()){
			recursive_add_from_dir(root);
		}
		else{
			for (auto &file : files_to_archive){
				if (!exists(file)){
					warning(fmt::format(tr_txt("Path {0} does not exist at {1}\n"), file, root));
					continue;
				}
				if (fs::is_directory(file))
					recursive_add_from_dir(file);
				else
					add(file);
			}
		}
		creator_->finish();
		next_->commit();
		catalog->add_fs_state(next);
	}
	catch(std::exception &e){
		string msg = fmt::format(tr_txt("Error while archiving {0}:\n"), name);
		msg += message(e);
		warning(move(msg));
	}
}

