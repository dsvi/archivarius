#pragma once
#include "piping.h"

class Pipe_xxhash_in : public Pipe_in
{
public:
	Pipe_xxhash_in();
	virtual
	Pump_result pump(ui8 *to, ui64 size) override;

	ui64 digest();
	void reset();
private:
	xxh::hash_state64_t state_;
};


class Pipe_xxhash_out : public Pipe_out
{
public:
	Pipe_xxhash_out();
	virtual
	void pump(ui8 *from, ui64 size) override;

	ui64 digest();
	void reset();
private:
	xxh::hash_state64_t state_;
};

