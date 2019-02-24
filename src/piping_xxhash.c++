#include "piping_xxhash.h"


Pipe_xxhash_in::Pipe_xxhash_in() : state_(0)
{

}

Source::Pump_result Pipe_xxhash_in::pump(u8 *to, u64 size)
{
	auto res = pump_next(to, size);
	state_.update(to, res.pumped_size);
	return res;
}

u64 Pipe_xxhash_in::digest()
{
	return state_.digest();
}

void Pipe_xxhash_in::reset()
{
	state_.reset(0);
}

Pipe_xxhash_out::Pipe_xxhash_out() : state_(0)
{

}

void Pipe_xxhash_out::pump(u8 *from, u64 size)
{
	state_.update(from, size);
	pump_next(from, size);
}

u64 Pipe_xxhash_out::digest()
{
	return state_.digest();
}

void Pipe_xxhash_out::reset()
{
	state_.reset(0);
}
