#pragma once
#include "precomp.h"

namespace archi{


struct Test_settings{
	std::string name; //optional
	std::filesystem::path archive_path;
	std::string password;
	std::function<void(std::string &&header, std::string &&warning_message)> warning;
};

void test(Test_settings &cfg);


}
