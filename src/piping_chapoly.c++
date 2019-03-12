#include "piping_chapoly.h"
#include "exception.h"

void Chapoly::inc_iv()
{
	for (auto &v : iv_){
		if (++v != 0)
			break;
	}
}

void Chapoly::set_key_word(std::string_view kw)
{
	if (kw.empty())
		throw Exception("password should not be empty");
}

Pipe_chapoly_out::Pipe_chapoly_out()
{

}

Pipe_chapoly_out::Pipe_chapoly_out(Chapoly &p)
{
	set_params(p);
}

void Pipe_chapoly_out::set_params(Chapoly &p)
{

}
