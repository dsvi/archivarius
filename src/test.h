#pragma once
#include "precomp.h"

class Progress_bar;

namespace archi{


struct Test_action{
	std::string name; //optional
	std::filesystem::path archive_path;
	std::string password;
	std::function<void(std::string &&header, std::string &&warning_message)> warning;
	std::function<void(std::string &&status_text)> progress_status;
	Progress_bar *progress = nullptr; // when set, test() sets its min/max and updates it in raw units: states checked, then bytes checked

	void test();
};



}
