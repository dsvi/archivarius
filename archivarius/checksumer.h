#pragma once
#include "checksum.h"

namespace archi{

class Checksumer
{
public:
	virtual
	Checksum checksum() =0;
	virtual
	void reset() =0;
	virtual
	void update(u8 *from, u64 size) =0;
	virtual
	~Checksumer(){};
};

}
