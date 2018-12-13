#pragma once
#include "precomp.h"

struct File_content_ref{
	std::string fname;
	u64 from;
	u64 to;
	u64 compressed_size; // space taken in file. never 0
	u64 ref_count_ = 0;  // only Catalogue can change this
};

inline
bool operator < (const File_content_ref &a, const File_content_ref &b){
	int cmp = a.fname.compare(b.fname);
	if ( cmp < 0 )
		return true;
	if ( cmp == 0 && a.from < b.from )
		return true;
	return false;
}
