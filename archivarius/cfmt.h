#pragma once

/*

Forground colors are coded as {fC} and backround ones as {bC},
where C is one of the color codes:

bk - black
r  - red
g  - green
b  - blue
y  - yellow
m  - magenta
c  - cyan
w  - white
d  - default color

Adding "b" before color codes makes them brighter. For example bright green foreground {fbg}

Styles are:
{i}Italic{ni}
{b}Bold{nb}
{u}Underlined{nu}
{s}Striken{ns}
{p}Pulsating (blinking){np}
{r}Reversed colors{nr}

{d} returns all styles to default
*/

#include <fmt/core.h>
#include <fmt/args.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

namespace cfmt{

inline
bool colorise_stdout(){
	#pragma warning(suppress : 4996) // MSVC warning
	static bool colorize = isatty(STDOUT_FILENO);
	return colorize;
}
inline
bool colorise_stderr(){
	#pragma warning(suppress : 4996) // MSVC warning
	static bool colorise = isatty(STDERR_FILENO);
	return colorise;
}


#define F_BK   fmt::arg("fbk", "\033[30m")
#define F_R    fmt::arg("fr",  "\033[31m")
#define F_G    fmt::arg("fg",  "\033[32m")
#define F_Y    fmt::arg("fy",  "\033[33m")
#define F_B    fmt::arg("fb",  "\033[34m")
#define F_M    fmt::arg("fm",  "\033[35m")
#define F_C    fmt::arg("fc",  "\033[36m")
#define F_W    fmt::arg("fw",  "\033[37m")
#define F_D    fmt::arg("fd",  "\033[39m")

#define F_BBK  fmt::arg("fbbk", "\033[90m")
#define F_BR   fmt::arg("fbr",  "\033[91m")
#define F_BG   fmt::arg("fbg",  "\033[92m")
#define F_BY   fmt::arg("fby",  "\033[93m")
#define F_BB   fmt::arg("fbb",  "\033[94m")
#define F_BM   fmt::arg("fbm",  "\033[95m")
#define F_BC   fmt::arg("fbc",  "\033[96m")
#define F_BW   fmt::arg("fbw",  "\033[97m")

#define B_BK   fmt::arg("bbk", "\033[40m")
#define B_R    fmt::arg("br",  "\033[41m")
#define B_G    fmt::arg("bg",  "\033[42m")
#define B_Y    fmt::arg("by",  "\033[43m")
#define B_B    fmt::arg("bb",  "\033[44m")
#define B_M    fmt::arg("bm",  "\033[45m")
#define B_C    fmt::arg("bc",  "\033[46m")
#define B_W    fmt::arg("bw",  "\033[47m")
#define B_D    fmt::arg("bd",  "\033[49m")

#define B_BBK  fmt::arg("bbbk", "\033[100m")
#define B_BR   fmt::arg("bbr",  "\033[101m")
#define B_BG   fmt::arg("bbg",  "\033[102m")
#define B_BY   fmt::arg("bby",  "\033[103m")
#define B_BB   fmt::arg("bbb",  "\033[104m")
#define B_BM   fmt::arg("bbm",  "\033[105m")
#define B_BC   fmt::arg("bbc",  "\033[106m")
#define B_BW   fmt::arg("bbw",  "\033[107m")

#define TS_D   fmt::arg("d",  "\033[0m")
//Text styles
#define TS_I   fmt::arg("i",  "\033[3m")
#define TS_NI  fmt::arg("ni", "\033[23m")
#define TS_B   fmt::arg("b",  "\033[1m")
#define TS_NB  fmt::arg("nb", "\033[22m")
#define TS_U   fmt::arg("u",  "\033[4m")
#define TS_NU  fmt::arg("nu", "\033[24m")
#define TS_S   fmt::arg("s",  "\033[9m")
#define TS_NS  fmt::arg("ns", "\033[29m")
#define TS_P   fmt::arg("p",  "\033[5m")
#define TS_NP  fmt::arg("np", "\033[25m")
#define TS_R   fmt::arg("r",  "\033[7m")
#define TS_NR  fmt::arg("nr", "\033[27m")

#define CFMT_ARG_LIST \
F_BK, F_R, F_G, F_Y, F_B, F_M, F_C, F_W, F_D, \
F_BBK, F_BR, F_BG, F_BY, F_BB, F_BM, F_BC, F_BW, \
B_BK, B_R, B_G, B_Y, B_B, B_M, B_C, B_W, B_D, \
B_BBK, B_BR, B_BG, B_BY, B_BB, B_BM, B_BC, B_BW, \
TS_D, TS_I, TS_NI, TS_B, TS_NB, TS_U, TS_NU, TS_S, TS_NS, TS_P, TS_NP, TS_R, TS_NR

#define DF_BK   fmt::arg("fbk", "")
#define DF_R    fmt::arg("fr",  "")
#define DF_G    fmt::arg("fg",  "")
#define DF_Y    fmt::arg("fy",  "")
#define DF_B    fmt::arg("fb",  "")
#define DF_M    fmt::arg("fm",  "")
#define DF_C    fmt::arg("fc",  "")
#define DF_W    fmt::arg("fw",  "")
#define DF_D    fmt::arg("fd",  "")

#define DF_BBK  fmt::arg("fbbk", "")
#define DF_BR   fmt::arg("fbr",  "")
#define DF_BG   fmt::arg("fbg",  "")
#define DF_BY   fmt::arg("fby",  "")
#define DF_BB   fmt::arg("fbb",  "")
#define DF_BM   fmt::arg("fbm",  "")
#define DF_BC   fmt::arg("fbc",  "")
#define DF_BW   fmt::arg("fbw",  "")

#define DB_BK   fmt::arg("bbk", "")
#define DB_R    fmt::arg("br",  "")
#define DB_G    fmt::arg("bg",  "")
#define DB_Y    fmt::arg("by",  "")
#define DB_B    fmt::arg("bb",  "")
#define DB_M    fmt::arg("bm",  "")
#define DB_C    fmt::arg("bc",  "")
#define DB_W    fmt::arg("bw",  "")
#define DB_D    fmt::arg("bd",  "")

#define DB_BBK  fmt::arg("bbbk", "")
#define DB_BR   fmt::arg("bbr",  "")
#define DB_BG   fmt::arg("bbg",  "")
#define DB_BY   fmt::arg("bby",  "")
#define DB_BB   fmt::arg("bbb",  "")
#define DB_BM   fmt::arg("bbm",  "")
#define DB_BC   fmt::arg("bbc",  "")
#define DB_BW   fmt::arg("bbw",  "")

#define DTS_D   fmt::arg("d",  "")

#define DTS_I   fmt::arg("i",  "")
#define DTS_NI  fmt::arg("ni", "")
#define DTS_B   fmt::arg("b",  "")
#define DTS_NB  fmt::arg("nb", "")
#define DTS_U   fmt::arg("u",  "")
#define DTS_NU  fmt::arg("nu", "")
#define DTS_S   fmt::arg("s",  "")
#define DTS_NS  fmt::arg("ns", "")
#define DTS_P   fmt::arg("p",  "")
#define DTS_NP  fmt::arg("np", "")
#define DTS_R   fmt::arg("r",  "")
#define DTS_NR  fmt::arg("nr", "")

#define CFMT_EMPTY_ARG_LIST \
DF_BK, DF_R, DF_G, DF_Y, DF_B, DF_M, DF_C, DF_W, DF_D, \
DF_BBK, DF_BR, DF_BG, DF_BY, DF_BB, DF_BM, DF_BC, DF_BW, \
DB_BK, DB_R, DB_G, DB_Y, DB_B, DB_M, DB_C, DB_W, DB_D, \
DB_BBK, DB_BR, DB_BG, DB_BY, DB_BB, DB_BM, DB_BC, DB_BW, \
DTS_D, DTS_I, DTS_NI, DTS_B, DTS_NB, DTS_U, DTS_NU, DTS_S, DTS_NS, DTS_P, DTS_NP, DTS_R, DTS_NR


template<class ...T>
void cprint(std::string_view fmt, T&&... args){
	using namespace fmt;
	if (colorise_stdout())
		vprint(fmt, make_format_args(std::forward<T>(args)..., CFMT_ARG_LIST) );
	else
		vprint(fmt, make_format_args(std::forward<T>(args)..., CFMT_EMPTY_ARG_LIST) );
}

template<class ...T>
void cprint(std::FILE *f, std::string_view fmt, T&&... args){
	using namespace fmt;
	bool colorise = true;
	if (f == stdout) {
		colorise = colorise_stdout();
	}
	else
	if (f == stderr) {
		colorise = colorise_stderr();
	}
	if (colorise)
		vprint(f, fmt, make_format_args(std::forward<T>(args)..., CFMT_ARG_LIST) );
	else
		vprint(f, fmt, make_format_args(std::forward<T>(args)..., CFMT_EMPTY_ARG_LIST) );
}

template<class ...T>
std::string cformat(std::string_view fmt, T&&... args){
	using namespace fmt;
	if (colorise_stdout())
		return cformat(fmt, make_format_args(std::forward<T>(args)..., CFMT_ARG_LIST) );
	else
		return cformat(fmt, make_format_args(std::forward<T>(args)..., CFMT_EMPTY_ARG_LIST) );
}

template<typename OutputIt, typename ...T>
OutputIt cformat_to(OutputIt out, std::string_view fmt, T&&... args){
	if (colorise_stdout())
		return cformat_to(out, fmt, make_format_args(std::forward<T>(args)..., CFMT_ARG_LIST) );
	else
		return cformat_to(out, fmt, make_format_args(std::forward<T>(args)..., CFMT_EMPTY_ARG_LIST) );
}

/// clears everything printed to begining of the cursor line, and sets the cursor to the begining
/// usefull for progress indicators
inline
void clear_to_begining_of_line(){
	if (colorise_stdout())
		fmt::print("\033[1K\033[1G");
}

/// moves the cursor to the begining of current line,
/// clearing everything in its path
/// usefull for progress indicators
inline
void move_cursor_to_begining_of_line(){
	if (colorise_stdout())
		fmt::print("\033[1G");
}

inline
void move_cursor_up(){
	if (colorise_stdout())
		fmt::print("\033[A");
}

}
