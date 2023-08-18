#pragma once
#include "file_content_ref.h"
#include "filesystem_state.h"
#include "platform.h"

namespace archi{


class Catalogue
{
public:
	Catalogue(std::filesystem::path &arc_path, std::string_view password, bool create_new);

	std::filesystem::path archive_path();

	void password(std::string_view password);

	// from newest to oldest
	auto state_times(){
		return fs_state_files_ | std::views::transform([](auto &fs){return fs.time_created;});
	}
	Time state_time(size_t ndx){
		ASSERT(ndx < fs_state_files_.size());
		return fs_state_files_[ndx].time_created;
	}

	size_t num_states();
	// 0 is the latest state. up to num_states()
	Filesystem_state fs_state(size_t ndx);
	Filesystem_state latest_fs_state(); //or empty fs_state if no states available
	Filesystem_state empty_fs_state();

	void add_fs_state(Filesystem_state &&fs);
	void remove_fs_state(Filesystem_state &&fs);

	// returns sorted
	auto content_refs(){
		return content_refs_ | std::views::all;
	}

	void commit();
private:

	std::set<File_content_ref> content_refs_;
	struct Fs_state_file{
		std::string name;
		Time        time_created;
		Filters_in  filters;
	};
	std::vector<Fs_state_file> fs_state_files_; // sorted from newest to oldest
	std::filesystem::path cat_file_;
	std::unique_ptr<File_lock> file_lock_;
	std::optional<Chapoly> enc_;

	// includes the catalogue filename itself.
	// basically files which are not in the returned set can be safely deleted.
	std::unordered_set<std::string> used_files();
	// removes everything which is not in used_files
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
size_t Catalogue::num_states()
{
	return fs_state_files_.size();
}


}
