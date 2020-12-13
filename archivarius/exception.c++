#include "exception.h"
#include "globals.h"

namespace archi{


Exception::Exception(Exception::Tag t)
{
	tag_ = t;
}

Exception::Exception(std::string &&error_message) : fmt_(error_message)
{

}

Exception::Exception(const char *fmt_string) :fmt_(tr_txt(fmt_string))
{

}

const char *Exception::what() const noexcept
{
	return fmt_.c_str();
}

std::string message(std::exception &e)
{
	std::string msg;
	try {
		msg += e.what();
		msg += '\n';
		std::rethrow_if_nested(e);
	} catch(std::exception& e) {
		msg += message(e);
	}
	return msg;
}

bool has_tag(std::exception &e, Exception::Tag t)
{
	if ( auto p = dynamic_cast<Exception*>(&e); p )
		if ( p->tag() == t )
			return true;
	try {
		std::rethrow_if_nested(e);
	} catch(std::exception& e) {
		return has_tag(e, t);
	}
	return false;
}


}
