#pragma once

#include "precomp.h"

struct Config_zstd{

};

struct Config_enc{
	std::string password;
};

struct Config{
	std::string name;
	std::filesystem::path archive;
	std::filesystem::path root; // common path for files_to_archive
	// paths to archive
	// if not set, then archive all from root (not including the root)
	std::vector<std::filesystem::path> files_to_archive;
	std::vector<std::filesystem::path> files_to_ignore;
	std::optional<uint64_t>	 max_storage_time_seconds;
	bool                 process_acl = false;
	std::optional<Config_zstd> zstd;
	std::optional<Config_enc>  enc;
	uint64_t min_content_file_size = 0;
};


std::vector<Config> read_config(std::string_view filepath = "");
