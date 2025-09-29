#pragma once

#include <format>
#include <print>

namespace coformat{

/// wheither to colorize.
/// by default output is only colorized if both stderr and stout are outputing into terminal.
/// when any of them is redirected to a file, colorization is disabled.
void colorize(bool v);
bool is_colorized();

/// clears the line above and sets the cursor to the begining of it.
/// does not take effect until '\n' is printed somehow
/// does nothing if is_colorized() returns false.
void clear_previous_line();

std::string cvformat( std::string_view fmt, std::format_args args );
std::string cvformat( const std::locale& loc, std::string_view fmt, std::format_args args );

template< class... Args >
std::string cformat( std::string_view fmt, Args&&... args ){
	return cvformat(fmt, std::make_format_args(args...));
}

template< class... Args >
std::string cformat( const std::locale& loc, std::string_view fmt, Args&&... args ){
	return cvformat(loc, fmt, std::make_format_args(args...));
}

template< class... Args >
void cprint( std::FILE* stream, std::string_view fmt, Args&&... args ){
	std::print(stream, "{}", cformat(fmt, std::forward<Args>(args)...));
}

template< class... Args >
void cprint(std::string_view fmt, Args&&... args ){
	std::print("{}", cformat(fmt, std::forward<Args>(args)...));
}

template< class... Args >
void cprintln( std::FILE* stream, std::string_view fmt, Args&&... args ){
	std::println(stream, "{}", cformat(fmt, std::forward<Args>(args)...));
}

template< class... Args >
void cprintln( std::string_view fmt, Args&&... args ){
	std::println("{}", cformat(fmt, std::forward<Args>(args)...));
}

}
