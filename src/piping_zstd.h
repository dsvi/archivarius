#pragma once
#include "piping.h"
#include "buffer.h"

class Pipe_zstd_in: public Pipe_in{
public:
	virtual
	Pump_result pump(u8 *to, u64 size) override;
private:
	Buffer buffer_;
};



class Pipe_zstd_out: public Pipe_out{
public:
	virtual
	void pump(u8 *to, u64 size) override;
private:
	Buffer buffer_;
};
