#include "checksumer_xxhash.h"

namespace archi {

Checksum Checksumer_xxhash::checksum()
{
	return state_.digest();
}

void Checksumer_xxhash::reset()
{
	state_.reset(0);
}

void Checksumer_xxhash::update(u8 *from, u64 size)
{
	state_.update(from, size);
}

}
