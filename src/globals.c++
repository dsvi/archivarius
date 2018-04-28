#include "globals.h"
#include "platform.h"
#include "exception.h"
#include "catalogue.h"

using namespace std;
namespace fs = filesystem;

//string exec(const char *cmd)
//{
//	array<char, 1024> buffer;
//	string result;
//	unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
//	if (!pipe)
//		throw std::runtime_error("popen() failed!");
//	while (fgets(&buffer.front(), buffer.size(), pipe.get()) != nullptr)
//		result += &buffer.front();
//	return result;
//}

const char *tr_text(const char *s)
{
	return s;
}

static
std::filesystem::file_time_type posix_epoch;

void init_epoch()
{
	struct tm start = {.tm_year=70, .tm_mon=0, .tm_mday=1, .tm_hour=0, .tm_min=0, .tm_sec=0};
	time_t t = mktime(&start);
	posix_epoch = std::filesystem::file_time_type::clock::from_time_t(t);
}


ui64 to_posix_time(std::filesystem::file_time_type time)
{
	return chrono::duration_cast<chrono::seconds>(time - posix_epoch).count();
}

std::filesystem::file_time_type from_posix_time(ui64 posix_seconds)
{
	return posix_epoch + std::chrono::seconds(posix_seconds);
}

string current_time_to_filename()
{
	time_t t = time(nullptr);
	if (t == -1)
		throw Exception("Can't get current time");
	auto st = gmtime(&t);
	ostringstream name;
	name << put_time(st, "%Y-%m-%d %H:%M:%S");
	return name.str();
}

std::tuple<Filesystem_state::File, bool> make_file(std::filesystem::path &file_path, std::filesystem::path &&archive_path)
{
	auto sts = symlink_status(file_path);
	Filesystem_state::File f;
	f.path = move(archive_path);
	f.unix_permissions = to_int(sts.permissions());
	auto type = sts.type();
	if (type == fs::file_type::regular)
		f.type = Filesystem_state::FILE;
	else if (type == fs::file_type::directory)
		f.type = Filesystem_state::DIR;
	else if (type == fs::file_type::symlink)
		f.type = Filesystem_state::SYMLINK;
	else
		return {f, false};
	f.mod_time = to_posix_time(last_write_time(file_path));
	if (f.type == Filesystem_state::SYMLINK)
		f.symlink_target = fs::read_symlink(file_path);
	f.acl = get_acl(file_path);
	if (f.type == Filesystem_state::DIR)
		f.default_acl = get_default_acl(file_path);
	return {f, true};
}



std::filesystem::path home_dir()
{
	char *env = std::getenv("HOME");
	if (env)
		return env;
	return fs::path();
}

void find_and_replace(string &where, const string &what, const string &replace_to)
{
	for(string::size_type i = 0; (i = where.find(what, i)) != string::npos;){
		where.replace(i, what.length(), replace_to);
		i += replace_to.length();
	}
}
