#pragma once
#include "piping.h"
#include "piping_chapoly.h"
#include "buffer.h"

class Chacha: public Encryption_params{};

class Pipe_chacha_in: public Pipe_in{
public:
	Pipe_chacha_in(Chacha &p);
private:
	virtual
	Pump_result pump(u8 *to, u64 size) override;
	Botan::ChaCha chacha_{20};
};



class Pipe_chacha_out: public Pipe_out{
public:
	Pipe_chacha_out(Chacha &p);
private:
	virtual
	void pump(u8 *to, u64 size) override;
	Botan::ChaCha chacha_{20};
};


