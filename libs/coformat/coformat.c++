#include <cctype>
#include <unordered_map>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

#include "coformat.h"


namespace coformat{

static bool runing_on_a_tty(){
	#ifdef _MSC_VER
	#pragma warning(suppress : 4996) // MSVC warning
	#endif
	return isatty(STDOUT_FILENO) and isatty(STDERR_FILENO);
}

static bool _colorize = runing_on_a_tty();

void colorize(bool v)
{
	_colorize = v;
}

bool is_colorized()
{
	return _colorize;
}

void clear_previous_line(){
	if (is_colorized())
		std::print("\033[F\033[K");
}

static std::unordered_map<std::string_view, std::string_view> _esc_codes = {
	{"fbk", "\033[30m"},
	{"fr",  "\033[31m"},
	{"fg",  "\033[32m"},
	{"fy",  "\033[33m"},
	{"fb",  "\033[34m"},
	{"fm",  "\033[35m"},
	{"fc",  "\033[36m"},
	{"fw",  "\033[37m"},
	{"fd",  "\033[39m"},

	{"fbbk", "\033[90m"},

	{"fbr",  "\033[91m"},
	{"fbg",  "\033[92m"},
	{"fby",  "\033[93m"},
	{"fbb",  "\033[94m"},
	{"fbm",  "\033[95m"},
	{"fbc",  "\033[96m"},
	{"fbw",  "\033[97m"},

	{"bbk", "\033[40m"},
	{"br",  "\033[41m"},
	{"bg",  "\033[42m"},
	{"by",  "\033[43m"},
	{"bb",  "\033[44m"},
	{"bm",  "\033[45m"},
	{"bc",  "\033[46m"},
	{"bw",  "\033[47m"},
	{"bd",  "\033[49m"},

	{"bbbk", "\033[100m"},
	{"bbr",  "\033[101m"},
	{"bbg",  "\033[102m"},
	{"bby",  "\033[103m"},
	{"bbb",  "\033[104m"},
	{"bbm",  "\033[105m"},
	{"bbc",  "\033[106m"},
	{"bbw",  "\033[107m"},

	{"d",  "\033[0m"},

	{"i",  "\033[3m"},
	{"ni", "\033[23m"},
	{"b",  "\033[1m"},
	{"nb", "\033[22m"},
	{"u",  "\033[4m"},
	{"nu", "\033[24m"},
	{"s",  "\033[9m"},
	{"ns", "\033[29m"},
	{"p",  "\033[5m"},
	{"np", "\033[25m"},
	{"r",  "\033[7m"},
	{"nr", "\033[27m"},
};

static
std::string substitute_wth_esc_codes(std::string_view str){
	using namespace std;

	string ret;
	string field;
	ret.reserve(str.size()*2);

	for (auto p = str.begin(); p != str.end(); p++){
		if (*p == '{'){
			if (++p == str.end())
				throw std::format_error("Unmatched '{' at the end of the format string");
			if (*p == '{'){
				ret += '{';
				ret += '{';
				continue;
			}
			auto old_p = p;
			auto throw_if_end = [&]{
				if (p == str.end())
					throw format_error(format("'{{' at position {} left unmached by '}}'", distance(str.begin(), old_p) -1));
			};
			if (isdigit(*p) or *p == ':' or *p == '}'){
				// normal format field. skip it till '}'
				ret += '{';
				size_t nested = 0;
				do {
					ret += *p;
					if (*p == '{')
						nested++;
					if (*p == '}'){
						if (nested == 0)
							break;
						nested--;
					}
				} while(++p != str.end());
				throw_if_end();
			}
			else {
				field.clear();
				do {
					field += *p;
				} while(++p != str.end() and *p != '}');
				throw_if_end();
				if (!_esc_codes.contains(field))
					throw format_error(format("Unrecognized style or color {{{}}}", field));
				if (_colorize)
					ret += _esc_codes.at(field);
			}
		}
		else
			if (*p == '}'){
				if (++p != str.end() and *p == '}'){
					ret += '}';
					ret += '}';
				}
				else
					throw format_error(format("Unexpected '}}' at position {}", distance(str.begin(), p) -1));
			}
			else
				ret += *p;
	}
	return ret;
}

std::string cvformat( std::string_view fmt, std::format_args args){
	return vformat(substitute_wth_esc_codes(fmt), args);
}

std::string cvformat( const std::locale& loc, std::string_view fmt, std::format_args args){
	return vformat(loc, substitute_wth_esc_codes(fmt), args);
}


}
