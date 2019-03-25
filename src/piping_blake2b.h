#pragma once
#include "checksum.h"
#include "piping.h"

class Blake2b_csumer : public Checksumer_base{
public:
	Checksum digest() override;
	void reset() override;
protected:
	Botan::Blake2b state_{512};
};

class Pipe_blake2b_in : public Pipe_in, public Blake2b_csumer
{
private:
	virtual
	Pump_result pump(u8 *to, u64 size) override;
};


class Pipe_blake2b_out : public Pipe_out, public Blake2b_csumer
{
private:
	virtual
	void pump(u8 *from, u64 size) override;
};

