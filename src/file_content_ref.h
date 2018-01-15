#pragma once
#include "precomp.h"

struct File_content_ref{
	std::string fname;
	ui64 from;
	ui64 to;
	ui64 xxhash;
	ui64 compressed_size; // equals to - from for non compressed files. never 0
	ui64 ref_count_ = 0;  // only Catalogue can change this
};

bool operator < (const File_content_ref &a, const File_content_ref &b){
	int cmp = a.fname.compare(b.fname);
	if ( cmp < 0 )
		return true;
	if ( cmp == 0 && a.from < b.from )
		return true;
	return false;
}
