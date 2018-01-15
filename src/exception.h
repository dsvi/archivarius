#pragma once
#include "precomp.h"

class Exception : public std::exception
{
public:
	explicit
	Exception(const char*fmt_string);
	template<class T>
	Exception & operator << (const T &t){
		fmt_ % t;
		return *this;
	}
	std::string message();
private:
	boost::format fmt_;
};

std::string message(std::exception &e);

