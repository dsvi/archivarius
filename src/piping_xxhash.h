#pragma once
#include "piping.h"

class Pipe_xxhash_in : public Pipe_in
{
public:
	Pipe_xxhash_in();
	u64 digest();
	void reset();
private:
	xxh::hash_state64_t state_;
	virtual
	Pump_result pump(u8 *to, u64 size) override;
};


class Pipe_xxhash_out : public Pipe_out
{
public:
	Pipe_xxhash_out();

	u64 digest();
	void reset();
private:
	xxh::hash_state64_t state_;
	virtual
	void pump(u8 *from, u64 size) override;
};

