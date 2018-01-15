#pragma once
#include "precomp.h"
#include "filesystem_state.h"
#include "file_content_creator.h"


std::string exec(const char *cmd);

const char* tr_text(const char *);

extern
std::function<void(std::string_view)> report_warning;

// example 2017-03-04 23:04:17
std::string current_time_to_filename();

void init_epoch();
ui64 to_posix_time(std::filesystem::file_time_type);
std::filesystem::file_time_type from_posix_time(ui64 posix_seconds);


// just add content_ref to it
std::tuple<Filesystem_state::File, bool> make_file(
    std::filesystem::directory_entry &entry,
    std::filesystem::path &&archive_path);

std::filesystem::path home_dir(); // might be empty
