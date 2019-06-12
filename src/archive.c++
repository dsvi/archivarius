#include "archive.h"
#include "globals.h"
#include "exception.h"
#include "catalogue.h"
#include "file_content_creator.h"

using namespace std;
namespace fs = std::filesystem;


std::tuple<Filesystem_state::File, bool> Archive_settings::make_file(const std::filesystem::path &file_path, std::filesystem::path &&archive_path)
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


void Archive_settings::recursive_add_from_dir(const fs::path &dir_path)
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

void Archive_settings::add(const fs::path &file_path)
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

void Archive_settings::archive()
{
	try{
		Catalogue cat(archive_path, password);
		catalog = &cat;
		auto prev = catalog->latest_fs_state();
		auto next = catalog->empty_fs_state();
		prev_ = &prev;
		next_ = &next;
		// -=- GC -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		force_to_archive.clear();
		if (max_storage_time){
			auto max_ref = cat.max_ref_count();
			if (max_ref != 0){
				struct Old_files{
					fs::path *path;
					string_view content_fn;
					u64      size;
				};
				vector<Old_files> old_enough_to_compact;
				for (Filesystem_state::File &file: prev.files()){
					if ( file.content_ref.has_value() && file.content_ref.value().ref_count_ == cat.max_ref_count()){
						auto &a = old_enough_to_compact.emplace_back();
						a.path = &file.path;
						a.content_fn = file.content_ref->fname;
						a.size = file.content_ref->space_taken;
					}
				}
				unordered_map<string_view, u64> content_file_waste;
				for (auto &f : old_enough_to_compact){
					if (content_file_waste.find(f.content_fn) != content_file_waste.end())
						continue;
					auto size = file_size(archive_path / f.content_fn);
					content_file_waste[f.content_fn] = max(size, min_content_file_size);
				}
				for (auto &f :old_enough_to_compact){
					auto &size = content_file_waste[f.content_fn];
					size -= min(f.size, size); // underflow protection
				}
				// now content_file_sizes contains wasted space for each file
				unordered_set<string_view> content_files_to_compact;
				for (auto &cz: content_file_waste){
					if (cz.second < min_content_file_size / 16)
						continue;   // too little is wasted
					content_files_to_compact.insert(cz.first);
				}
				u64 total_size = 0;
				for (auto &f : old_enough_to_compact ){
					if (content_files_to_compact.find(f.content_fn) == content_files_to_compact.end())
						continue;
					force_to_archive.insert(*f.path);
					total_size += f.size;
				}
				if (total_size < min_content_file_size) //not enough even for one new content file
					force_to_archive.clear();
			}
		}

		auto fcc = File_content_creator(archive_path);
		fcc.min_file_size(min_content_file_size);
		creator_ = &fcc;
		if (!password.empty())
			creator_->enable_encryption();
		if (zstd)
			creator_->enable_compression(*zstd);
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
				add(file);
				if (fs::is_directory(file))
					recursive_add_from_dir(file);
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
		if (max_storage_time){
			try{
				auto t = to_posix_time(fs::file_time_type::clock::now()) - *max_storage_time;
				vector<Filesystem_state> states_to_remove;
				size_t ndx = 0;
				for (auto state_time : cat.state_times()){
					if (state_time < t)
						states_to_remove.push_back(cat.fs_state(ndx));
					ndx++;
				}
				// leave at least one state
				if (cat.max_ref_count() == states_to_remove.size())
					states_to_remove.erase(states_to_remove.begin());
				for (auto &fs_state : states_to_remove)
					cat.remove_fs_state(fs_state);
			}
			catch(std::exception &exp){
				warning( tr_txt("Error while remoing old state"), message(exp) );
			}
		}
		catalog->commit();
	}
	catch(std::exception &e){
		warning(fmt::format(tr_txt("Error while archiving {0}:"), name), message(e));
	}
}

