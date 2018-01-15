#pragma once
#include "precomp.h"

struct Restore_settings{
	std::string name;
	std::filesystem::path from;
	int from_ndx;
	std::filesystem::path to;
	std::function<void(std::string&&)> warning;
};

void restore(Restore_settings &cfg);
