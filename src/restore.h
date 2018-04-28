#pragma once
#include "precomp.h"

struct Restore_settings{
	std::string name; //optional
	std::filesystem::path from;
	size_t from_ndx;
	std::filesystem::path to;
	std::function<void(std::string&&)> warning;
};

void restore(Restore_settings &&cfg);
