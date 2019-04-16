#pragma once
#include "precomp.h"
#include "catalogue.h"

struct Restore_settings{
	std::string name; //optional
	size_t from_ndx;
	std::filesystem::path to;
	Catalogue *cat;
	std::function<void(std::string&&)> warning;
};

void restore(Restore_settings &cfg);
