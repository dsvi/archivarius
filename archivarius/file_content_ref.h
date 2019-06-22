#pragma once
#include "precomp.h"
#include "filters.h"
#include "checksum.h"

namespace archi{


struct File_content_ref{
	Filters_in filters;
	std::string fname;
	u64 from;
	u64 to;
	u64 space_taken; // space taken in file. never 0
	Checksum csum;
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

static_assert (std::is_nothrow_move_constructible<File_content_ref>::value);

}
