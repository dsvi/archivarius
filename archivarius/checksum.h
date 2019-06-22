#pragma once
#include "precomp.h"

namespace archi{


typedef u64 Xx_hash;
typedef std::array<u8, 64> Blake2b_hash;

typedef std::variant<Xx_hash, Blake2b_hash> Checksum;

inline
bool operator == (const Checksum &a, const Checksum &b){
	using namespace std;
	if ( holds_alternative<Xx_hash>(a) ){
		ASSERT(holds_alternative<Xx_hash>(b));
		return get<Xx_hash>(a) == get<Xx_hash>(b);
	}
	if ( holds_alternative<Blake2b_hash>(a) ){
		ASSERT(holds_alternative<Blake2b_hash>(b));
		return get<Blake2b_hash>(a) == get<Blake2b_hash>(b);
	}
	return false;
}

inline
bool operator != (const Checksum &a, const Checksum &b){
	return !(a == b);
}

class Checksumer_base
{
public:
	virtual
	Checksum digest() =0;
	virtual
	void reset() =0;
};


}

