#pragma once
#include "precomp.h"
#include "filesystem_state.h"
#include "file_content_creator.h"


//std::string exec(const char *cmd);
/// translate message to the choosen locale
const char* tr_txt(const char *);

void find_and_replace(std::string& where, const std::string &what, const std::string &replace_to);
void trim(std::string &s);

// example 2017-03-04 23:04:17
std::string current_time_to_filename();

void init_epoch();
u64 to_posix_time(std::filesystem::file_time_type);
std::filesystem::file_time_type from_posix_time(u64 posix_seconds);


// just add content_ref to it
std::tuple<Filesystem_state::File, bool> make_file(
    const std::filesystem::path &entry,
          std::filesystem::path &&archive_path);

std::filesystem::path home_dir(); // might be empty
