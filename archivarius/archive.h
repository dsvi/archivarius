#pragma once
#include "precomp.h"
#include "file_content_creator.h"
#include "catalogue.h"

namespace archi{

class Archive_settings{
public:
	std::string name;
	std::filesystem::path archive_path;
	std::filesystem::path root;
	std::vector<std::filesystem::path> files_to_archive; // if not set, then archive all from root (not including the root)
	std::unordered_set<std::filesystem::path> files_to_exclude;
	u64 min_content_file_size;
	std::optional<Time> max_storage_time;
	std::string password;
	std::optional<Zstd_out> zstd;
	std::function<void(std::string &&header, std::string &&warning_message)> warning;
	bool process_acls;
private:
	void archive();
	void add(const std::filesystem::path &file_path);
	void recursive_add_from_dir(const std::filesystem::path &dir_path);
	// just add content_ref to it
	std::tuple<Filesystem_state::File, bool> make_file(
	    const std::filesystem::path &entry,
	          std::filesystem::path &&archive_path);

	std::unordered_set<std::filesystem::path> force_to_archive;// relative to archive_path. list of files to 'compact'
	Catalogue *catalog;
	File_content_creator *creator_;
	Filesystem_state *prev_;
	Filesystem_state *next_;
	friend void archive(Archive_settings a);
};

inline
void archive(Archive_settings a){
	a.archive();
}


}
