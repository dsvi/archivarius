#pragma once
#include "piping_xxhash.h"
#include "piping_blake2b.h"


namespace archi{


class Checksumer
{
public:
	Checksumer();
	Checksumer(Checksumer &) = delete;
	Checksumer& operator= (Checksumer &) = delete;
	/// sets up piping to produce the same checksum type as \p csum
	void set_for(const Checksum &csum);
	Pipe_out &pipe();
	void reset();
	Checksum checksum();
private:
	std::variant<Pipe_xxhash_out, Pipe_blake2b_out> cs_pipe_holder_;
	Checksumer_base *cs_;
	Pipe_out *cs_pipe_;
};

inline
Pipe_out &Checksumer::pipe()
{
	return *cs_pipe_;
}

inline
void Checksumer::reset()
{
	return cs_->reset();
}

inline
Checksum Checksumer::checksum()
{
	return cs_->digest();
}


}
