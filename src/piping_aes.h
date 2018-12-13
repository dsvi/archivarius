#pragma once
#include "piping.h"

class Pipe_aes_in: public Pipe_in{
public:
	virtual
	Pump_result pump(u8 *to, u64 size) override;
private:
	//std::vector<ui64> buffer_;
};



class Pipe_aes_out: public Pipe_out{
public:
	virtual
	void pump(u8 *to, u64 size) override;
private:
	//std::vector<ui64> buffer_;
};


