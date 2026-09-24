#pragma once
#include "precomp.h"

class Progress_bar;

namespace archi{

struct Restore_action{
	std::string name; // optional
	std::filesystem::path archive_path;
	size_t from_ndx;
	std::filesystem::path to;
	std::string password;
	std::filesystem::path prefix; // optional
	std::function<void(std::string &&header, std::string &&warning_message)> warning;
	Progress_bar *progress = nullptr; // when set, restore() sets its min/max and updates it in bytes written

	void restore();
};


}
