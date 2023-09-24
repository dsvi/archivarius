#pragma once
#include "precomp.h"

namespace archi{


class Exception : public std::exception
{
public:
	struct Tag{
		Tag(){
			p = this;
		}
		bool operator == (Tag a){
			return p == a.p;
		}
	private:
		void *p;
	};
	explicit
	Exception(Tag t);
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
	Exception &tag(Tag t);
	Tag  tag();
private:
	std::string fmt_;
	Tag tag_;
};


inline
Exception &Exception::tag(Exception::Tag t)
{
	tag_ = t;
	return *this;
}

inline
Exception::Tag Exception::tag()
{
	return tag_;
}

// checks this and all the nested
bool has_tag(std::exception &e, Exception::Tag t);
std::string message(std::exception &e);


}
