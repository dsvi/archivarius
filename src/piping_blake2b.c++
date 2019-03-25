#include "piping_blake2b.h"

Checksum Blake2b_csumer::digest()
{
	Blake2b_hash ret;
	state_.final(ret.data());
	return ret;
}

void Blake2b_csumer::reset()
{
	state_.clear();
}

Source::Pump_result Pipe_blake2b_in::pump(u8 *to, u64 size)
{
	auto r = pump_next(to, size);
	state_.update(to, size);
	return r;
}

void Pipe_blake2b_out::pump(u8 *from, u64 size)
{
	state_.update(from, size);
	pump_next(from, size);
}

