#pragma once
#include "precomp.h"

namespace archi{


struct Test_action{
	std::string name; //optional
	std::filesystem::path archive_path;
	std::string password;
	std::function<void(std::string &&header, std::string &&warning_message)> warning;
	std::function<void(std::string &&status_text)> progress_status;
	std::function<void(uint progress_in_permil)> progress;

	void test();
};



}
