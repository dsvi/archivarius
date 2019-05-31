#pragma once
#include "precomp.h"
#include "filesystem_state.h"
#include "file_content_creator.h"


//std::string exec(const char *cmd);
/// translate message to the choosen locale
const char* tr_txt(const char *);

void find_and_replace(std::string& where, const std::string &what, const std::string &replace_to);
void trim(std::string &s);

void init_epoch();

// returns unique filename in /p dir
std::filesystem::path make_unique_filename(const std::filesystem::path &dir, std::string_view prefix);

Time to_posix_time(std::filesystem::file_time_type);
std::filesystem::file_time_type from_posix_time(Time t);

std::filesystem::path home_dir(); // might be empty
