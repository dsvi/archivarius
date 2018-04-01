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
	enum File_type: ui8{
		FILE,
		DIR,
		SYMLINK,
	};

	struct File {
		std::filesystem::path path;
		ui16        unix_permissions;
		File_type   type;
		ui64        mod_time; // in posix seconds
		std::optional<File_content_ref> content_ref; // only for regular files with sizes > 0
		std::filesystem::path	symlink_target;
		std::string acl; // posix long format
		std::string default_acl; // posix long format
	};
	// creates empty state
	explicit
	Filesystem_state(const std::filesystem::path &arc_path);
	// loads state from disc
	Filesystem_state(const std::filesystem::path &arc_path, std::string_view name, ui64 time_created_posix);

	void add(File &&f);

	std::string_view file_name();
	ui64 time_created();

	std::optional<File_content_ref> get_ref_if_exist(
	                                                std::filesystem::path &archive_path,
	                                                ui64 modified_seconds );

	// for (File &file: fss.files())...
	auto files() -> decltype( std::declval<std::unordered_map<std::filesystem::path,File>&>() | boost::adaptors::map_values){
		return files_ | boost::adaptors::map_values;
	}

	void commit();
private:
	// key is pathname
	std::unordered_map<std::filesystem::path, File> files_;
	std::string filename_;
	std::filesystem::path arc_path_;
	ui64 time_created_;
};

