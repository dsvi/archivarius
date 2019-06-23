#include "exception.h"
#include "globals.h"

namespace archi{


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

std::string_view Exception::message()
{
	return fmt_;
}

std::string message(std::exception &e)
{
	std::string msg;
	try {
		Exception *my_exp = dynamic_cast<Exception*>(&e);
		if (my_exp)
			msg += my_exp->message();
		else
			msg += e.what();
		msg += '\n';
		std::rethrow_if_nested(e);
	} catch(std::exception& e) {
		msg += message(e);
	}
	return msg;
}


}