#include "piping_xxhash.h"


Pipe_xxhash_in::Pipe_xxhash_in() : state_(0)
{

}

Source::Pump_result Pipe_xxhash_in::pump(ui8 *to, ui64 size)
{
	auto res = next_->pump(to, size);
	state_.update(to, res.pumped_size);
	return res;
}

ui64 Pipe_xxhash_in::digest()
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

void Pipe_xxhash_out::pump(ui8 *from, ui64 size)
{
	state_.update(from, size);
	next_->pump(from, size);
}

ui64 Pipe_xxhash_out::digest()
{
	return state_.digest();
}

void Pipe_xxhash_out::reset()
{
	state_.reset(0);
}
