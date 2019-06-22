#include "globals.h"
#include "platform.h"
#include "exception.h"
#include "catalogue.h"

using namespace std;
namespace fs = filesystem;

namespace archi{


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

const char *tr_txt(const char *s)
{
	return s;
}

static
std::filesystem::file_time_type posix_epoch;

void init_epoch()
{
	//TODO: switch to c++20 ?
	tm start;
	start.tm_year=70;
	start.tm_mon=0;
	start.tm_mday=1;
	start.tm_hour=0;
	start.tm_min=0;
	start.tm_sec=0;
	time_t t = mktime(&start);
	posix_epoch = std::filesystem::file_time_type::clock::from_time_t(t);
}

Time to_posix_time(std::filesystem::file_time_type time)
{
	return chrono::duration_cast<chrono::nanoseconds>(time - posix_epoch).count();
}

std::filesystem::file_time_type from_posix_time(Time t)
{
	return posix_epoch + std::chrono::nanoseconds(t);
}

fs::path make_unique_filename(const filesystem::path &dir, string_view prefix)
{
	time_t t = time(nullptr);
	if (t == -1)
		throw Exception("Can't get current time");
	auto st = localtime(&t);
	ostringstream name;
	name << put_time(st, "%Y-%m-%d %H:%M:%S");
	fs::path file;
	fs::path fname;
	size_t count = 0;
	do {
		fname = prefix;
		fname += name.str();
		if (count++)
			fname += string("#") + to_string(count -2);
		file = dir;
		file /= fname;
	}while (fs::exists(file));
	return fname;
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


}
