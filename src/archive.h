#pragma once
#include "precomp.h"
#include "file_content_creator.h"


struct Archive_settings{
	std::string name;
	std::filesystem::path archive;
	std::vector<std::filesystem::path> files_to_archive;
	std::function<void(std::string&&)> warning;
	File_content_creator &creator;
};

void archive(Archive_settings &cfg);


