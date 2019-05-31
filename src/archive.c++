#include "archive.h"
#include "globals.h"
#include "exception.h"
#include "catalogue.h"
#include "file_content_creator.h"

using namespace std;
namespace fs = std::filesystem;


std::tuple<Filesystem_state::File, bool> Archiver::make_file(const std::filesystem::path &file_path, std::filesystem::path &&archive_path)
{
	auto sts = symlink_status(file_path);
	Filesystem_state::File f;
	f.path = move(archive_path);
	auto type = sts.type();
	if (type == fs::file_type::regular)
		f.type = Filesystem_state::FILE;
	else if (type == fs::file_type::directory)
		f.type = Filesystem_state::DIR;
	else if (type == fs::file_type::symlink)
		f.type = Filesystem_state::SYMLINK;
	else
		return {f, false};
	if (f.type != Filesystem_state::SYMLINK){
		f.unix_permissions = to_int(sts.permissions());
		f.mod_time = to_posix_time(last_write_time(file_path));
		if (process_acls){
			f.acl = get_acl(file_path);
			if (f.type == Filesystem_state::DIR)
				f.default_acl = get_default_acl(file_path);
		}
	}
	else
		f.symlink_target = fs::read_symlink(file_path);
	return {f, true};
}


void Archiver::recursive_add_from_dir(const fs::path &dir_path)
try {
	std::vector<fs::path> dirs;
	for (auto &e : fs::directory_iterator(dir_path)){
		auto p = e.path();
		if (files_to_exclude.find(p) != files_to_exclude.end())
			continue;
		add(p);
		if (e.is_directory() and !e.is_symlink())
			dirs.push_back(p);
	}
	for (auto &dir : dirs)
		recursive_add_from_dir(dir);
}
catch(std::exception &exp){
	warning(fmt::format(tr_txt("Can't get directory contents for {0}:"), dir_path), message(exp));
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
				ASSERT(file.mod_time);
				file.content_ref = prev_->get_ref_if_exist(file.path, *file.mod_time);
				// TODO: switch to c++2a contains
				if (!file.content_ref or force_to_archive.find(file.path) != force_to_archive.end())
					file.content_ref = creator_->add(file_path);
			}
		}
		next_->add(move(file));
	}
	catch(std::exception &exp){
		warning(fmt::format(tr_txt("Skipping {0}:"), file_path), message(exp));
	}
}

void Archiver::archive()
{
	try{
		auto fcc = File_content_creator(archive_path);
		fcc.min_file_size(min_content_file_size);
		creator_ = &fcc;
		if (encryption)
			creator_->enable_encryption();
		if (zstd)
			creator_->enable_compression(*zstd);
		auto prev = catalog->latest_fs_state();
		auto next = catalog->empty_fs_state();
		prev_ = &prev;
		next_ = &next;
		if (!root.empty()){
			for (auto &file : files_to_archive)
				file = root / file;
			decltype(files_to_exclude) tmp;
			for (auto &file : files_to_exclude)
				tmp.insert(root / file);
			files_to_exclude = tmp;
		}
		if (files_to_archive.empty()){
			recursive_add_from_dir(root);
		}
		else{
			for (auto &file : files_to_archive){
				if (!exists(file)){
					warning(fmt::format(tr_txt("Path {0} does not exist"), file), "");
					continue;
				}
				if (fs::is_directory(file))
					recursive_add_from_dir(file);
				else
					add(file);
			}
		}
		creator_->finish();
		if (zstd){
			auto cs = creator_->compression_statistic();
			if (cs.original){
				auto percent = cs.compressed *100 / cs.original;
				fmt::print("Archive compressed to {}% of original size\n", percent);
			}
		}
		next_->commit();
		catalog->add_fs_state(next);
	}
	catch(std::exception &e){
		warning(fmt::format(tr_txt("Error while archiving {0}:"), name), message(e));
	}
}

