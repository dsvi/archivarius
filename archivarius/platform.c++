#include "platform.h"
#include "exception.h"

#include <sys/acl.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
namespace fs = filesystem;

namespace archi{


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
std::string get_acl_internal(const std::filesystem::path &path, acl_type_t type)
{
	try{
		errno = 0;
		acl_ptr acl( acl_get_file(path.c_str(), type), acl_free);
		if (errno == ENOTSUP or errno == ENODATA)
			return string();
		check_error();
		acl_txt_ptr acl_text( acl_to_text(acl.get(), NULL), acl_free);
		check_error();
		return string((char*)acl_text.get());
	}
	catch(...){
		throw_with_nested( Exception("Can't get ACL for {0}")(path) );
	}
}

std::string get_acl(const std::filesystem::path &path)
{
	return get_acl_internal(path, ACL_TYPE_ACCESS);
}

string get_default_acl(const std::filesystem::path &path)
{
	return get_acl_internal(path, ACL_TYPE_DEFAULT);
}

void set_acl_internal(std::filesystem::path &path, const char* acl_txt, acl_type_t type)
{
	try{
		errno = 0;
		acl_ptr acl( acl_from_text(acl_txt), acl_free );
		check_error();
		acl_set_file(path.c_str(), type, acl.get());
		check_error();
	}
	catch(...){
		throw_with_nested( Exception("Can't set ACL for {0}")(path) );
	}
}

void set_acl(std::filesystem::path &path, string &acl_txt)
{
	set_acl_internal(path, acl_txt.c_str(), ACL_TYPE_ACCESS);
}

void set_default_acl(std::filesystem::path &path, string &acl_txt)
{
	set_acl_internal(path, acl_txt.c_str(), ACL_TYPE_DEFAULT);
}

class File_lock_int : public File_lock{
public:
	File_lock_int(std::filesystem::path &path){
		try{
			errno = 0;
			file_ = open(path.c_str(), O_RDWR);
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
			fcntl(file_, F_SETLK, &fl);
			check_error();
		}
		catch(...){
			throw_with_nested( Exception("Can't acquire file lock for {0}")(path) );
		}
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


}
