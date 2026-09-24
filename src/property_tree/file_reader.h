#pragma once
#include "../precomp.h"

namespace archi{

class File_reader{
public:
	File_reader() = default;
	File_reader(const File_reader&) = delete;
	File_reader &operator =(const File_reader&) = delete;
	File_reader(File_reader &&other) noexcept;
	File_reader &operator =(File_reader &&other) noexcept;
	~File_reader();

	/// Opens the file for reading. Binary mode always: the bytes are kept exactly as they are.
	/// Throws an Exception if the file can't be opened.
	static File_reader open(const std::filesystem::path &filepath);

	/// Closes the file; the object becomes empty whatever the result is.
	/// Doesn't throw, since closing is a routine part of a destructor: a failure
	/// aborts a debug build (ASSERT) and is ignored in a release one.
	void close() noexcept;

	bool is_open() const;
	explicit
	operator bool() const;
	std::FILE *handle() const;
	/// The name this handle was opened from. Empty for an empty File_reader.
	const std::filesystem::path &filename() const;

	/// fread(). Returns the amount of bytes read. Throws if the read failed.
	u64 read(u8 *to, u64 size);

	/// the ending '\n' is not stored in `line`, and a '\0' byte inside the line is kept as a regular character.
	/// Returns false when nothing has been read, because the stream is at eof.
	/// Throws if the stream reports an error.
	bool read_line(std::string &line);

	/// feof()
	bool eof();

private:
	File_reader(std::FILE *file, std::filesystem::path filename);

	/// Throws an Exception telling that `operation` has failed on this file.
	/// The reason is taken from the errno set by the failed function.
	[[noreturn]]
	void throw_error(std::string_view operation) const;

	std::FILE *file_ = nullptr;
	std::filesystem::path filename_;
};

inline
bool File_reader::is_open() const
{
	return file_ != nullptr;
}

inline
File_reader::operator bool() const
{
	return file_ != nullptr;
}

inline
std::FILE *File_reader::handle() const
{
	return file_;
}

inline
const std::filesystem::path &File_reader::filename() const
{
	return filename_;
}

inline
bool File_reader::eof()
{
	return feof(file_);
}

}
