#pragma once
#include "precomp.h"
#include "file_content_ref.h"
#include "stream.h"
#include "piping_xxhash.h"

class File_content_creator
{
public:
	explicit
	File_content_creator(const std::filesystem::path &arc_path);

	void min_file_size(u64 bytes);
	u64 min_file_size();

	/// @param file_name full path to file, which content will be added to archive
	File_content_ref add(const std::filesystem::path &file_name);

	void finish();
	static std::string prefix;
private:
	std::filesystem::path arc_path_;
	std::string   fname_;
	Stream_in     in_;
	Stream_out    out_;
	File_sink    file_;
	Pipe_xxhash_out cs_;
	u64 bytes_pumped_;
	u64 min_file_size_;
	Buffer buff_;

	void create_file();
};

inline
void File_content_creator::min_file_size(u64 bytes)
{
	min_file_size_ = bytes;
}

inline
u64 File_content_creator::min_file_size()
{
	return min_file_size_;
}

