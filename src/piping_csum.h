#pragma once
#include "checksumer.h"
#include "piping.h"

namespace archi{

class Pipe_through_csumer{
public:
	Checksumer *csumer(){
		return csumer_.get();
	}
	void csumer(std::unique_ptr<Checksumer> &&cs);
	void csumer_for(const Checksum &csum);
private:
	std::unique_ptr<Checksumer> csumer_;
};


class Pipe_csum_in : public Pipe_in, public Pipe_through_csumer{
public:
	Pipe_csum_in() =default;
	explicit
	Pipe_csum_in(const Checksum set_for);
	Pipe_csum_in(std::unique_ptr<Checksumer> &&cs);
private:
	virtual
	Pump_result pump(u8 *to, u64 size) override;
};


class Pipe_csum_out : public Pipe_out, public Pipe_through_csumer{
public:
	Pipe_csum_out() =default;
	explicit
	Pipe_csum_out(const Checksum set_for);
	Pipe_csum_out(std::unique_ptr<Checksumer> &&cs);
private:
	virtual
	void pump(u8 *from, u64 size) override;
};


}
