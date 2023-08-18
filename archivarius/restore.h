#pragma once
#include "precomp.h"
#include "catalogue.h"

namespace archi{

struct Restore_action{
	std::string name; // optional
	std::filesystem::path archive_path;
	size_t from_ndx;
	std::filesystem::path to;
	std::string password;
	std::filesystem::path prefix; // optional
	std::function<void(std::string &&header, std::string &&warning_message)> warning;
	std::function<void(uint progress_in_permil)> progress;

	void restore();
};



}
