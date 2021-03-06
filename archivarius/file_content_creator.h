#pragma once
#include "precomp.h"
#include "exception.h"
#include "file_content_ref.h"
#include "stream.h"
#include "filters.h"
#include "checksumer.h"

namespace archi{


class File_content_creator
{
public:
	explicit
	File_content_creator(const std::filesystem::path &arc_path);

	void enable_compression(Zstd_out &p);
	void enable_encryption();

	void min_file_size(u64 bytes);
	u64 min_file_size();

	/// @param file_name full path to file, which content will be added to archive
	File_content_ref add(const std::filesystem::path &file_name);

	void finish();
	struct Compression_ratio{
		u64 original;
		u64 compressed; // can be 0
	};
	Compression_ratio compression_statistic();

	// when error happened at output ( add() or finish() ), we dont know what had already reached the disk.
	// so it's better to abort the whole thing
	static
	Exception::Tag unrecoverable_output_problem;
private:
	std::filesystem::path arc_path_;
	std::string   fname_;
	Stream_in     in_;
	Stream_out    out_;
	File_sink     file_sink_;
	Checksumer    cs_;
	u64 bytes_pumped_;
	u64 min_file_size_;
	Buffer buff_;
	Filtrator_out filters_;
	std::optional<Chacha> enc_;
	Compression_ratio comp_ratio_{0,0};

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

inline
File_content_creator::Compression_ratio File_content_creator::compression_statistic()
{
	return comp_ratio_;
}


}
