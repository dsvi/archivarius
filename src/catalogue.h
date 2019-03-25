#pragma once
#include "file_content_ref.h"
#include "filesystem_state.h"
#include "platform.h"
#include "globals.h"

class Catalogue
{
public:
	Catalogue(std::filesystem::path &arc_path, std::string_view key);

	std::filesystem::path archive_path();

	void key(std::string_view key);
	//std::vector<std::filesystem::file_time_type> state_times();
	auto  state_times(){
		return fs_state_files_ | ranges::view::transform([](auto &fs){return from_posix_time(fs.time_created);});
	}
	// ndx from array returned by state_times()
	Filesystem_state fs_state(size_t ndx);
	Filesystem_state latest_fs_state(); //or empty fs_state if no states available
	Filesystem_state empty_fs_state();

	void add_fs_state(Filesystem_state &fs);
	void remove_fs_state(Filesystem_state &fs);

	// == number of state files
	u64 max_ref_count();

	void commit();
private:

	std::set<File_content_ref> content_refs_;
	struct Fs_state_file{
		std::string name;
		u64         time_created;
		Filters_in  filters;
	};
	std::vector<Fs_state_file> fs_state_files_;
	std::filesystem::path cat_file_;
	std::unique_ptr<File_lock> file_lock_;
	std::optional<Chapoly> enc_;

	// includes the catalogue filename itself.
	// basically files which are not in the returned set can be safely deleted.
	std::unordered_set<std::string> used_files();
	// remoes everything which is not in used_files
	void clean_up();
	void throw_inconsistent(uint line);
	File_content_ref map_ref(File_content_ref &r);
};

static_assert (std::is_nothrow_move_constructible<Catalogue>::value);

inline
std::filesystem::path Catalogue::archive_path()
{
	return cat_file_.parent_path();
}

inline
u64 Catalogue::max_ref_count()
{
	return fs_state_files_.size();
}
