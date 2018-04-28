#include "cmd_line_parser.h"
#include "exception.h"

using namespace std;

Cmd_line parse_command_line(int argc, char *argv[])
{
	try {
		Cmd_line ret;
		ASSERT(argc > 1);
		ret.cmd_ = argv[1];
		for(int i = 2; i < argc; i++){
			string pstr = argv[i];
			auto eq_pos = pstr.find("=");
			if (eq_pos == pstr.npos)
				throw Exception("Wrong parameter format %1%. Should be param=val") << pstr;
			string pname = pstr.substr(0, eq_pos);
			if (pname.empty())
				throw Exception("Wrong parameter format %1%. Should be param=val") << pstr;
			string pval  = pstr.substr(eq_pos + 1, pstr.size());
			if (pval.empty())
				throw Exception("No value for '%1%'") << pname;
			ret.params_[pname] = pval;
		}
		return ret;
	} catch (...) {
		throw_with_nested(Exception("Command line parsing error."));;
	}
}

std::optional<std::string_view> Cmd_line::param_str(std::string_view name)
{
	auto n = std::string(name);
	if (auto it = params_.find(n); it == params_.end())
		return {};
	else
		return it->second;
}

std::optional<uint> Cmd_line::param_uint(std::string_view name)
{
	auto str = param_str(name);
	if (!str)
		return {};
	try{
		return stoul(string(str.value()));
	}
	catch(...){
		throw_with_nested(Exception("Parameter '%1%' must be unsigned integer") << name);
	}
}

std::optional<bool> Cmd_line::param_bool(std::string_view name)
{
	auto str = param_str(name);
	if (!str)
		return {};
	string s{ str.value() };
	for (auto &i : s)
		i = tolower(i);
	if ( s == "on")
		return {true};
	if ( s == "off")
		return {false};
	throw Exception("value for '%1%' must be either 'on' or 'off'") << name;
}

std::string_view Cmd_line::param_str(std::string_view name)
{
	auto s = param_str_opt(name);
	if (!s)
		throw Exception("Required parameter '%1%' missing") << name;
	return s.value();
}
