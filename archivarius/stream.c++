#include "stream.h"
#include "exception.h"
#include "globals.h"

using namespace std;

namespace archi{


Stream_in::Stream_in()
{
}

Stream_in::Stream_in(string &&name)
{
	name_ = move(name);
}

u64 Stream_in::get_uint()
{
	u64 v = 0;
	u8 sv;
	size_t shift = 0;
	do{
		Source::Pump_result read = pump(&sv, 1);
		if (read.pumped_size != 1)
			throw Exception( "Malformed file: {0}" )(name_);
		if (shift > sizeof(v) * 8 - 7)
			throw Exception( "Too big varint. Malformed file {0}" )(name_);
		v |= (sv &127) << shift;
		shift += 7;
	}while(sv &128);
	return v;
}

u64 Stream_in::get_uint64()
{
	u64 v;
	auto res = pump((u8*) &v, sizeof(v));
	if (res.pumped_size != sizeof(v))
		throw Exception( "Malformed file: {0}" )(name_);
	return v;
}

Source::Pump_result Stream_in::pump(u8 *to, u64 size)
{
	try{
		return pump_next(to, size);
	}
	catch(...){
		throw_with_nested( Exception("Can't read the file {0}")(name_));
	}
}

Stream_out::Stream_out()
{

}

Stream_out::Stream_out(std::string &&name)
{
	name_ = move(name);
}

void Stream_out::pump(u8 *from, u64 size)
{
	try{
		if (size)
			pump_next(from, size);
	}
	catch(exception &){
		throw_default_error();
	}
}

void Stream_out::finish()
{
	try{
		finish_next();
	}
	catch(exception &){
		throw_default_error();
	}
}

void Stream_out::throw_default_error()
{
	throw_with_nested( Exception( "Error writing file {0}" )(name_).tag(error_tag_) );
}

void Stream_out::put_uint(u64 v)
{
	buff_.resize(0);
	do{
		u8 sv = v & 127;
		v >>= 7;
		sv |= 128;
		buff_.push_back(sv);
	}while (v);
	buff_.back() &= 127;
	pump(&buff_.front(), buff_.size());
}

void Stream_out::put_uint64(u64 v)
{
	pump((u8*)&v, sizeof(v));
}

void read_message(Buffer &message, Stream_in &in, Checksumer_xxhash &cs)
{
	auto msize = in.get_uint();
	message.resize(msize);
	cs.reset();
	auto res = in.pump(message.raw(), msize);
	if (res.pumped_size != msize)
		throw Exception("Malformed file: {0}")(in.name());
	auto cs_now = cs.checksum();
	auto cs_was = in.get_uint64();
	if (cs_now != cs_was)
		throw Exception("Control sums don't match. Corrupted file.");
}

void put_message(google::protobuf::MessageLite &msg, Buffer &buff, Stream_out &out, Checksumer_xxhash &csr)
{
	auto msize = msg.ByteSizeLong();
	buff.resize(msize);
	msg.SerializeToArray(buff.raw(), msize);
	out.put_uint(msize);
	csr.reset();
	out.pump(buff.raw(), msize);
	auto cs = csr.checksum();
	out.put_uint64(get<Xx_hash>(cs));
}


}
