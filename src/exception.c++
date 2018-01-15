#include "exception.h"
#include "globals.h"

Exception::Exception(const char *fmt_string) :fmt_(tr_text(fmt_string))
{

}

std::string Exception::message()
{
	return fmt_.str();
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

