#include "piping_chapoly.h"
#include "exception.h"

Pipe_chapoly_in::Pipe_chapoly_in(Chapoly &p)
{
	decryptor_.set_key(p.key(), p.key_size());
	decryptor_.start(p.iv(), p.iv_size());
}

Source::Pump_result Pipe_chapoly_in::pump(u8 *to, u64 size)
{
	if (buf_.empty()){
		try{
			buf_.resize(50'000'000);
			size_t offset = 0;
			while(true){
				auto res = pump_next(buf_.data() + offset, buf_.size() - offset);
				if (res.eof){
					buf_.resize(offset + res.pumped_size);
					break;
				}
				ASSERT(res.pumped_size == buf_.size() - offset);
				offset = buf_.size();
				buf_.resize(buf_.size() * 3/2);
			}
			decryptor_.finish(buf_);
			buf_.shrink_to_fit();
		} catch(Botan::Integrity_Failure &ef){
			throw Exception("ChaCha20Poly1305 integrity check failed. The file was altered or damaged.");
		}
	}
	auto reminder = buf_.size() - offset_;
	if (size > reminder)
		size = reminder;
	std::copy_n(buf_.data() + offset_, size, to);
	offset_ += size;
	Source::Pump_result res;
	if (offset_ == buf_.size())
		res.eof = true;
	else
		res.eof = false;
	res.pumped_size = size;
	return res;
}


Pipe_chapoly_out::Pipe_chapoly_out(Chapoly &p)
{
	encryptor_.set_key(p.key(), p.key_size());
	encryptor_.start(p.iv(), p.iv_size());
}


void Pipe_chapoly_out::pump(u8 *to, u64 size)
{
	buf_.insert(buf_.end(), to, to+size);
}

void Pipe_chapoly_out::finish()
{
	auto granularity = encryptor_.update_granularity();
	auto till_top = granularity - buf_.size() % granularity;
	buf_.insert(buf_.end(), till_top, 0);
	encryptor_.finish(buf_);
	pump_next(buf_.data(), buf_.size());
}

