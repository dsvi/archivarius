#include "piping_chacha.h"
#include "exception.h"

namespace archi{


Pipe_chacha_in::Pipe_chacha_in(Chacha &p)
{
	chacha_.set_key(p.key(), p.key_size());
	chacha_.set_iv(p.iv(), p.iv_size());
}

Source::Pump_result Pipe_chacha_in::pump(u8 *to, u64 size)
{
	auto res = pump_next(to, size);
	chacha_.cipher1(to, res.pumped_size);
	return res;
}

Pipe_chacha_out::Pipe_chacha_out(Chacha &p)
{
	chacha_.set_key(p.key(), p.key_size());
	chacha_.set_iv(p.iv(), p.iv_size());
}

void Pipe_chacha_out::pump(u8 *to, u64 size)
{
	chacha_.cipher1(to, size);
	pump_next(to, size);
}


}
