#pragma once
#include "precomp.h"

std::string get_acl(std::filesystem::path &path);
std::string get_default_acl(std::filesystem::path &path);
void set_acl(std::filesystem::path &path, std::string acl_txt);
void set_default_acl(std::filesystem::path &path, std::string acl_txt);

class File_lock{
public:
	File_lock(){};
	File_lock(File_lock &) = delete;
	File_lock(File_lock &&) = default;
	virtual
	~File_lock() = 0;
};
// file at @path should already exist
std::unique_ptr<File_lock> lock(std::filesystem::path &path);

void fs_sync();
