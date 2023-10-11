#pragma once
#include "precomp.h"
#include "file_content_ref.h"
#include "filters.h"

namespace archi{


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
		File_type  type;
		std::optional<Time>   mod_time;
		std::optional<File_content_ref> content_ref; // only for regular files with sizes > 0
		std::filesystem::path	symlink_target;
		std::string acl; // posix long format
		std::string default_acl; // posix long format
		std::optional<u16>    unix_permissions;
	};

	void add(File &&f);

	std::string_view file_name();
	Time time_created();

	std::optional<File_content_ref> get_ref_if_exist(
	                                                std::filesystem::path &path_in_archive,
	                                                Time modified_time );

	// for (File &file: fss.files())...
	auto files(){
		return files_ | std::views::values;
	}

	Filters_in filters();

	void commit();
private:
	// key is pathname
	std::unordered_map<std::filesystem::path, File> files_;
	std::string filename_;
	std::filesystem::path arc_path_;
	Time time_created_;
	Filtrator_out filtrator_;

	// Only Catalogue allaws to create fstates
	friend class Catalogue;
	// creates empty state
	Filesystem_state(const std::filesystem::path &arc_path, Filters_out &f);
	// loads state from disc
	Filesystem_state(
	    const std::filesystem::path &arc_path,
	    std::string_view name,
	    Time time_created,
	    Filters_in &f,
	    std::function<File_content_ref(File_content_ref&)> ref_mapper);
};

static_assert (std::is_nothrow_move_constructible<Filesystem_state>::value);

inline
std::string_view Filesystem_state::file_name()
{
	return filename_;
}

inline
Time Filesystem_state::time_created()
{
	return time_created_;
}

inline
Filters_in Filesystem_state::filters()
{
	return filtrator_.get_filters();
}


}
