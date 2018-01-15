#pragma once
#include "file_content_ref.h"
#include "filesystem_state.h"
#include "platform.h"

class Catalogue
{
public:
	Catalogue(std::filesystem::path &arc_path);

	std::vector<std::filesystem::file_time_type> state_times();
	// ndx from array returned by state_times()
	Filesystem_state fs_state(size_t ndx);
	Filesystem_state latest_fs_state(); //or empty fs_state

	void add_fs_state(Filesystem_state &fs);
	void remove_fs_state(Filesystem_state &fs);

	ui64 max_ref_count();

	void commit();
private:

	std::set<File_content_ref> content_refs_;
	struct Fs_state_file{
		std::string name;
		ui64        time_created;
		bool        aes_encripted = false;
		bool        zstd_compressed = false;
	};
	std::vector<Fs_state_file> fs_state_files_;

	std::filesystem::path cat_file_;

	void add_ref(File_content_ref &ref);
	void remove_ref(File_content_ref &ref);

	std::unique_ptr<File_lock> file_lock_;

	// includes the catalogue filename itself.
	// basically files which are not in the returned set can be safely deleted.
	std::unordered_set<std::string> used_files();
	// remoes everything which is not in used_files
	void clean_up();
	void throw_inconsistent();
};



inline
ui64 Catalogue::max_ref_count()
{
	return fs_state_files_.size();
}
