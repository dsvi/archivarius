#pragma once
#include "precomp.h"
#include "file_content_creator.h"
#include "catalogue.h"

class Archiver{
public:
	std::string name;
	std::filesystem::path archive_path;
	std::filesystem::path root; // common path for files_to_archive
	std::vector<std::filesystem::path> files_to_archive; // if not set, then archive all from root (not including the root)
	std::function<void(std::string&&)> warning;
	Catalogue *catalog;
	void archive();
private:
	void add(const std::filesystem::path &file_path);
	void recursive_add_from_dir(const std::filesystem::path &dir_path);

	File_content_creator *creator_;
	Filesystem_state *prev_;
	Filesystem_state *next_;
};



