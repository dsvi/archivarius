#pragma once
#include "precomp.h"
#include "catalogue.h"

namespace archi{

struct Restore_settings{
	std::string name; //optional
	std::filesystem::path archive_path;
	size_t from_ndx;
	std::filesystem::path to;
	std::string password;
	std::function<void(std::string&&)> warning;
};

void restore(Restore_settings &cfg);


}
