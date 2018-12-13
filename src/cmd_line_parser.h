#pragma once
#include "precomp.h"
/**
 * @brief Parses command lines of the format "prog command param1=val param2=val"
 */
class Cmd_line{
public:
	std::string_view    command();
	void                check_unused_arguments();
	std::string         param_str(std::string_view name);
	std::optional<std::string> param_str_opt(std::string_view name);
	std::optional<uint> param_uint_opt(std::string_view name);
	std::optional<bool> param_bool_opt(std::string_view name);
private:
	std::string cmd_;
	std::unordered_map<std::string, std::string> params_;
	friend Cmd_line parse_command_line(int argc, char *argv[]);
};

Cmd_line parse_command_line(int argc, char *argv[]);

inline
std::string_view Cmd_line::command()
{
	return cmd_;
}
