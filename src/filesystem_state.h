#pragma once
#include "precomp.h"
#include "file_content_ref.h"

/*
Describes everything about files, except their contentents. Holds refs to contents for that.
Stored as s<date> files
*/
class Filesystem_state
{
public:
	enum File_type: u8{
		FILE,
		DIR,
		SYMLINK,
	};

	struct File {
		std::filesystem::path path;
		u16        unix_permissions;
		File_type  type;
		u64        mod_time; // in posix seconds
		std::optional<File_content_ref> content_ref; // only for regular files with sizes > 0
		std::filesystem::path	symlink_target;
		std::string acl; // posix long format
		std::string default_acl; // posix long format
	};
	// creates empty state
	explicit
	Filesystem_state(const std::filesystem::path &arc_path);
	// loads state from disc
	Filesystem_state(const std::filesystem::path &arc_path, std::string_view name, u64 time_created_posix);

	void add(File &&f);

	std::string_view file_name();
	u64 time_created();

	std::optional<File_content_ref> get_ref_if_exist(
	                                                std::filesystem::path &archive_path,
	                                                u64 modified_seconds );

	// for (File &file: fss.files())...
	auto files(){
		return files_ | ranges::view::values;
	}

	void commit();
private:
	// key is pathname
	std::unordered_map<std::filesystem::path, File> files_;
	std::string filename_;
	std::filesystem::path arc_path_;
	u64 time_created_;
};

inline
std::string_view Filesystem_state::file_name()
{
	return filename_;
}

inline
u64 Filesystem_state::time_created()
{
	return time_created_;
}


