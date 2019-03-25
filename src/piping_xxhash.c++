#include "piping_xxhash.h"


Checksum Xxhash_csumer::digest()
{
	return state_.digest();
}

void Xxhash_csumer::reset()
{
	state_.reset(0);
}

Source::Pump_result Pipe_xxhash_in::pump(u8 *to, u64 size)
{
	auto res = pump_next(to, size);
	state_.update(to, res.pumped_size);
	return res;
}

void Pipe_xxhash_out::pump(u8 *from, u64 size)
{
	state_.update(from, size);
	pump_next(from, size);
}

