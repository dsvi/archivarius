#include "archive.h"
#include "globals.h"
#include "exception.h"
#include "catalogue.h"
#include "file_content_creator.h"

using namespace std;
namespace fs = std::filesystem;

namespace archi{

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
	if (has_tag(exp, File_content_creator::unrecoverable_output_problem))
		throw;
	warning(fmt::format(tr_txt("Can't get directory contents for {0}:"), dir_path), message(exp));
}

void Archive_settings::add(const fs::path &file_path)
{
	try{
		Filesystem_state::File file;
		auto path_for_archive = root.empty() ? file_path : file_path.lexically_relative(root);
		file.path = move(path_for_archive);
		auto sts = symlink_status(file_path);
		auto type = sts.type();
		if (type == fs::file_type::regular)
			file.type = Filesystem_state::FILE;
		else if (type == fs::file_type::directory)
			file.type = Filesystem_state::DIR;
		else if (type == fs::file_type::symlink){
			file.type = Filesystem_state::SYMLINK;
			file.symlink_target = fs::read_symlink(file_path);
		} else
			return;
		if (file.type != Filesystem_state::SYMLINK){
			file.unix_permissions = to_int(sts.permissions());
			file.mod_time = to_posix_time(last_write_time(file_path));
			if (process_acls){
				file.acl = get_acl(file_path);
				if (file.type == Filesystem_state::DIR)
					file.default_acl = get_default_acl(file_path);
			}
			if (file.type == Filesystem_state::FILE){
				auto sz = fs::file_size(file_path);
				if (sz != 0){
					if (force_to_archive.contains(file.path))
						file.content_ref = long_term_content_->add(file_path);
					else {
						ASSERT(file.mod_time);
						file.content_ref = prev_->get_ref_if_exist(file.path, *file.mod_time);
						if (!file.content_ref)
							file.content_ref = normal_content_->add(file_path);
					}
				}
			}
		}
		next_->add(move(file));
	}
	catch(std::exception &exp){
		if (has_tag(exp, File_content_creator::unrecoverable_output_problem))
			throw;
		warning(fmt::format(tr_txt("Skipping {0}:"), file_path), message(exp));
	}
}

void Archive_settings::archive()
{
	try{
		Catalogue cat(archive_path, password, true);
		catalog = &cat;
		auto prev = catalog->latest_fs_state();
		auto next = catalog->empty_fs_state();
		prev_ = &prev;
		next_ = &next;
		// -=- GC -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		force_to_archive.clear();
		if (max_storage_time){
			auto max_ref = cat.num_states();
			if (max_ref != 0){
				struct Old_files{
					fs::path *path;
					string_view content_fn;
					u64 space_taken;
				};
				vector<Old_files> old_enough_to_compact;
				for (Filesystem_state::File &file: prev.files()){
					if ( file.content_ref.has_value() && file.content_ref.value().ref_count_ == max_ref){
						auto &a = old_enough_to_compact.emplace_back();
						a.path = &file.path;
						a.content_fn = file.content_ref->fname;
						a.space_taken = file.content_ref->space_taken;
					}
				}
				unordered_map<string_view, u64> content_file_waste;
				for (auto &f : old_enough_to_compact){
					if (content_file_waste.contains(f.content_fn))
						continue;
					auto size = file_size(archive_path / f.content_fn);
					content_file_waste[f.content_fn] = max(size, min_content_file_size);
				}
				for (auto &f :old_enough_to_compact){
					auto &size = content_file_waste[f.content_fn];
					size -= min(f.space_taken, size); // underflow protection
				}
				// now content_file_waste contains wasted space for each file
				u64 total_waste=0;
				unordered_set<string_view> content_files_to_compact;
				for (auto &cz: content_file_waste){
					if (cz.second < min_content_file_size / 16)
						continue;   // too little is wasted
					content_files_to_compact.insert(cz.first);
					total_waste += cz.second;
				}
				u64 total_size = 0;
				for (auto &f : old_enough_to_compact ){
					if (not content_files_to_compact.contains(f.content_fn))
						continue;
					force_to_archive.insert(*f.path);
					total_size += f.space_taken;
				}
				if (total_size < min_content_file_size and total_waste < 10*min_content_file_size){
					//not enough even for one new content file, and less then 10 content files are wasted
					force_to_archive.clear();
				}
			}
		}

		auto fccn = File_content_creator(archive_path);
		fccn.min_file_size(min_content_file_size);
		normal_content_ = &fccn;
		auto fccl = File_content_creator(archive_path);
		fccl.min_file_size(min_content_file_size);
		long_term_content_ = &fccl;

		if (!password.empty()){
			long_term_content_->enable_encryption();
			normal_content_->enable_encryption();
		}
		if (zstd){
			long_term_content_->enable_compression(*zstd);
			normal_content_->enable_compression(*zstd);
		}
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
		long_term_content_->finish();
		normal_content_->finish();
		// TODO: get rid of
		if (zstd){
			auto cs = normal_content_->compression_statistic();
			if (cs.original){
				auto percent = cs.compressed *100 / cs.original;
				fmt::print("Archive compressed to {}% of original size\n", percent);
			}
		}
		next.commit();
		catalog->add_fs_state(move(next));
		if (max_storage_time){
			try{
				auto t = to_posix_time(fs::file_time_type::clock::now()) - *max_storage_time;
				size_t num_states_to_remove = 0;
				ASSERT(cat.num_states());
				for (size_t i = cat.num_states(); --i > 0;){ // leave at least one state
					if (cat.state_time(i) < t)
						num_states_to_remove++;
					else
						break;
				}
				ASSERT(cat.num_states() > num_states_to_remove);
				while (num_states_to_remove-- > 0)
					cat.remove_fs_state(cat.fs_state(cat.num_states() -1));
			}
			catch(std::exception &exp){
				warning( tr_txt("Error while removing old state"), message(exp) );
				throw; // rethrow because catalog might be in inconsistent state
			}
		}
		catalog->commit();
	}
	catch(std::exception &e){
		warning(fmt::format(tr_txt("Error while archiving {0}:"), name), message(e));
	}
}

}
