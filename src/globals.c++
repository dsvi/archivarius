#include "globals.h"
#include "exception.h"

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

// for future translations
const char *tr_txt(const char *s)
{
	return s;
}


fs::path make_unique_filename(const filesystem::path &dir, string_view prefix)
{
	time_t t = time(nullptr);
	if (t == -1)
		throw Exception("Can't get current time");
	string name = format("{:%g-%m-%d %H:%M:%S}", chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()));
	fs::path file;
	fs::path fname;
	size_t count = 0;
	do {
		fname = prefix;
		fname += name;
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
