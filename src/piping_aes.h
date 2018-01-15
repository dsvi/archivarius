#pragma once
#include "piping.h"

class Pipe_aes_in: public Pipe_in{
public:
	virtual
	Pump_result pump(ui8 *to, ui64 size) override;
private:
	//std::vector<ui64> buffer_;
};



class Pipe_aes_out: public Pipe_out{
public:
	virtual
	void pump(ui8 *to, ui64 size) override;
private:
	//std::vector<ui64> buffer_;
};


