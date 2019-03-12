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
	std::optional<u64>	 max_storage_time;
	bool                 process_acl = false;
	std::optional<Config_zstd> zstd;
	std::optional<Config_enc>  enc;
};


std::vector<Config> read_config();
