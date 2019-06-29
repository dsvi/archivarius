#pragma once
#include "precomp.h"

namespace archi{


class Exception : public std::exception
{
public:
	explicit
	Exception(std::string &&error_message);
	explicit
	Exception(const char*fmt_string);
	template<class... Args>
	Exception operator()(const Args&... a){
		fmt_ = fmt::format(fmt_, a...);
		return *this;
	}
	const char *what() const noexcept override;
private:
	std::string fmt_;
};

std::string message(std::exception &e);


}
