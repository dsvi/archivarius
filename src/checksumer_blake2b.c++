#include "checksumer_blake2b.h"

namespace archi{

Checksum Checksumer_blake2b::checksum()
{
	Blake2b_hash ret;
	state_.final(ret.data());
	return ret;
}

void Checksumer_blake2b::reset()
{
	state_.clear();
}

void Checksumer_blake2b::update(u8 *from, u64 size)
{
	state_.update(from, size);
}

}
