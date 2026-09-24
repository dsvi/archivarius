#include "file_reader.h"
#include "../exception.h"

using namespace std;
namespace fs = std::filesystem;

namespace archi{


File_reader::File_reader(std::FILE *file, fs::path filename) : file_(file), filename_(move(filename))
{

}

File_reader::File_reader(File_reader &&other) noexcept : file_(other.file_), filename_(move(other.filename_))
{
	other.file_ = nullptr;
}

File_reader &File_reader::operator =(File_reader &&other) noexcept
{
	if (this != &other){
		close();
		file_ = other.file_;
		filename_ = move(other.filename_);
		other.file_ = nullptr;
	}
	return *this;
}

File_reader::~File_reader()
{
	close();
}

void File_reader::close() noexcept
{
	if (!file_)
		return;
	auto f = file_;
	file_ = nullptr;
	errno = 0;
	// fclose() frees the handle even when it reports an error
	[[maybe_unused]] auto err = fclose(f);
	// a destructor can't throw, so an error stays a trap for debug builds only
	ASSERT(0 == err);
}

File_reader File_reader::open(const fs::path &filepath)
{
	errno = 0;
	auto f = fopen(filepath.c_str(), "rb");
	if (!f)
		File_reader(nullptr, filepath).throw_error("Can't open for reading the file");
	return File_reader(f, filepath);
}

void File_reader::throw_error(string_view operation) const
{
	auto err = errno;
	errno = 0;
	string msg(operation);
	msg += " '";
	msg += filename_.string();
	msg += "': ";
	msg += err ? strerror(err) : "unknown error";
	throw Exception(move(msg));
}

u64 File_reader::read(u8 *to, u64 size)
{
	errno = 0;
	auto got = fread(to, 1, size, file_);
	if (ferror(file_))
		throw_error("Can't read from the file");
	return got;
}

bool File_reader::read_line(string &line)
{
	// getline(3) over the FILE*. Unlike fgets() it doesn't cut the line on a '\0'
	// byte inside it, and it takes a line of any length.
	static thread_local char *buf = nullptr;
	static thread_local size_t buf_size = 0;
	errno = 0;
	auto len = ::getline(&buf, &buf_size, file_);
	if (len == -1){
		if (ferror(file_))
			throw_error("Can't read a line from the file");
		return false; // the stream is at the end
	}
	while (len > 0 and buf[len - 1] == '\n')
		len--; // std::getline() alike: the ending '\n' is not stored
	line.assign(buf, len);
	return true;
}

}
