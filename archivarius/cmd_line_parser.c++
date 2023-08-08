#include "cmd_line_parser.h"
#include "exception.h"
#include "globals.h"

using namespace std;
using namespace fmt;
using namespace archi;

Cmd_line parse_command_line(int argc, const char *argv[])
{
	try {
		Cmd_line ret;
		ASSERT(argc > 1);
		ret.cmd_ = argv[1];
		for(int i = 2; i < argc; i++){
			string pstr = argv[i];
			auto eq_pos = pstr.find("=");
			if (eq_pos == pstr.npos || eq_pos == 0)
				throw Exception("Wrong parameter format {0}. Should be param=val")(pstr);
			string pname = pstr.substr(0, eq_pos);
			ASSERT(!pname.empty());
			string pval  = pstr.substr(eq_pos + 1, pstr.size());
			if (pval.empty())
				throw Exception("No value for '{0}'")(pname);
			ret.params_[pname] = pval;
		}
		return ret;
	} catch (...) {
		throw_with_nested(Exception("Command line parsing error."));;
	}
}

std::optional<std::string> Cmd_line::param_str_opt(std::string_view name)
{
	auto n = std::string(name);
	auto it = params_.find(n);
	if (it == params_.end())
		return {};
	auto ret = move(it->second);
	params_.erase(it);
	return ret;
}

std::optional<uint> Cmd_line::param_uint_opt(std::string_view name)
{
	auto str = param_str_opt(name);
	if (!str)
		return {};
	try{
		return stoul(string(str.value()));
	}
	catch(...){
		throw_with_nested(Exception("Parameter '{0}' must be unsigned integer")(name));
	}
}

std::optional<bool> Cmd_line::param_bool_opt(std::string_view name)
{
	auto str = param_str_opt(name);
	if (!str)
		return {};
	string s{ str.value() };
	for (auto &i : s)
		i = tolower(i);
	if ( s == "on")
		return {true};
	if ( s == "off")
		return {false};
	throw Exception("value for '{0}' must be either 'on' or 'off'")(name);
}

void Cmd_line::check_unused_arguments()
{
	if (params_.empty())
		return;
	string msg = format(tr_txt("Following command line parameters are superfluous: "));
	for (auto [key,v]: params_){
		msg += key;
		msg += ' ';
	}
	throw Exception(move(msg));
}

std::string Cmd_line::param_str(std::string_view name)
{
	auto s = param_str_opt(name);
	if (!s)
		throw Exception("Required parameter '{0}' missing")(name);
	return s.value();
}
