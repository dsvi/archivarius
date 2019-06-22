#pragma once
#include "piping.h"
#include "checksum.h"

namespace archi{


class Xxhash_csumer : public Checksumer_base{
public:
	Checksum digest() override;
	void reset() override;
protected:
	xxh::hash_state64_t state_{0};
};

class Pipe_xxhash_in : public Pipe_in, public Xxhash_csumer
{
private:
	virtual
	Pump_result pump(u8 *to, u64 size) override;
};


class Pipe_xxhash_out : public Pipe_out, public Xxhash_csumer
{
private:
	virtual
	void pump(u8 *from, u64 size) override;
};


}
