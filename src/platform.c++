#include "platform.h"

#include <sys/acl.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
namespace fs = filesystem;

void check_error(){
	if (!errno)
		return;
	auto err = errno;
	errno = 0;
	throw std::runtime_error(strerror(err));
}

typedef unique_ptr<remove_pointer_t<acl_t>, decltype(&acl_free)> acl_ptr;
typedef unique_ptr<char, decltype(&acl_free)> acl_txt_ptr;

static
std::string get_acl_internal(std::filesystem::path &path, acl_type_t type)
{
	// TODO wrap with trycatch for better messages
	errno = 0;
	acl_ptr acl( acl_get_file(path.c_str(), type), acl_free);
	check_error();
	acl_txt_ptr acl_text( acl_to_text(acl.get(), NULL), acl_free);
	check_error();
	return string((char*)acl_text.get());
}

std::string get_acl(std::filesystem::path &path)
{
	return get_acl_internal(path, ACL_TYPE_ACCESS);
}

string get_default_acl(std::filesystem::path &path)
{
	return get_acl_internal(path, ACL_TYPE_DEFAULT);
}


void set_acl(std::filesystem::path &path, string acl_txt)
{
	//TODO:
}

void set_default_acl(std::filesystem::path &path, string acl_txt)
{
	//TODO:
}

class File_lock_int : public File_lock{
public:
	File_lock_int(std::filesystem::path &path){
		errno = 0;
		file_ = open(path.c_str(), O_RDONLY);
		check_error();
		struct flock fl;
		memset(&fl, 0, sizeof(fl));

		// lock exclusively
		fl.l_type = F_WRLCK;
		// lock entire file
		fl.l_whence = SEEK_SET; // offset base is start of the file
		fl.l_start = 0;         // starting offset is zero
		fl.l_len = 0;           // len is zero, which is a special value representing end
		                        // of file (no matter how large the file grows in future)

		// F_SETLKW specifies blocking mode
		fcntl(file_, F_SETLKW, &fl);
		check_error();
	}

	~File_lock_int(){
		[[maybe_unused]]
		auto ret = close(file_);
		ASSERT(ret >= 0);
	};
private:
	decltype(open("",0)) file_;
};


std::unique_ptr<File_lock> lock(std::filesystem::path &path)
{
	ASSERT(fs::is_regular_file(path));
	return std::unique_ptr<File_lock>(new File_lock_int(path));
}

void fs_sync()
{
	sync();
}
