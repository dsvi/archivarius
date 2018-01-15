#include "stream.h"
#include "exception.h"
#include "globals.h"

using namespace std;


Stream_in::Stream_in()
{
}

Stream_in::Stream_in(string &&name)
{
	name_ = move(name);
}

ui64 Stream_in::get_uint()
{
	ui64 v = 0;
	ui8 sv;
	size_t num_bytes = 0;
	do{
		Source::Pump_result read = pump(&sv, 1);
		if (read.pumped_size != 1)
			throw Exception( "Malformed file or wrong password: %1%" ) << name_;
		v <<= 7;
		v |= sv &127;
		if (++num_bytes > sizeof(v) + 1 + (sizeof(v)-1)/8)
			throw Exception( "Too big varint. Malformed file or wrong password? %1%" ) << name_;
	}while(sv &128);
	return v;
}

Source::Pump_result Stream_in::pump(ui8 *to, ui64 size)
{
	try{
		return next_->pump(to, size);
	}
	catch(...){
		throw_with_nested( Exception("Can't read the file %1%") << name_);
	}
}

Stream_out::Stream_out(std::string &&name)
{
	name_ = move(name);
}

void Stream_out::pump(ui8 *from, ui64 size)
{
	try{
		next_->pump(from, size);
	}
	catch(...){
		throw_with_nested( Exception( "Error writing file %1%" ) << name_ );
	}
}

void Stream_out::put_uint(ui64 v)
{
	buff_.resize(0);
	do{
		ui8 sv = v & 127;
		v >>= 7;
		if (v)
			sv |= 128;
		buff_.push_back(sv);
	}while (v);
	pump(&buff_.front(), buff_.size());
}



void read_message(Buffer &message, Stream_in &in, Pipe_xxhash_in &cs_pipe)
{
	auto msize = in.get_uint();
	cs_pipe.reset();
	auto res = in.pump(message.raw(), msize);
	if (res.pumped_size != msize)
		throw Exception("Malformed file or wrong password: %1%") << in.name();
	auto cs_now = cs_pipe.digest();
	auto cs_was = in.get_uint();
	if (cs_now != cs_was)
		throw Exception("Control sums don't match. Corrupted file or wrong password.");
}

void put_message(google::protobuf::MessageLite &msg, Buffer &buff, Stream_out &out, Pipe_xxhash_out &cs_pipe)
{
	auto msize = msg.ByteSizeLong();
	buff.resize(msize);
	msg.SerializeToArray(buff.raw(), msize);
	out.put_uint(msize);
	cs_pipe.reset();
	out.pump(buff.raw(), msize);
	auto cs = cs_pipe.digest();
	out.put_uint(cs);
}
