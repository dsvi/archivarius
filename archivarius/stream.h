#pragma once
#include "precomp.h"
#include "piping.h"
#include "piping_xxhash.h"
#include "buffer.h"

namespace archi{


class Stream_in: public Pipe_in
{
public:
	Stream_in();
	explicit
	Stream_in(std::string &&name);

	void name(std::string &&name);
	std::string_view name();

	/// var length encoding
	u64 get_uint(); // ... or die trying
	/// not var length version. always takes 8 bytes in the stream
	u64 get_uint64(); // ... or die trying
	Source::Pump_result pump(u8 *to, u64 size) override;
private:
	std::string name_;
	std::vector<u8> buff_;
};

class Stream_out: public Pipe_out
{
public:
	Stream_out();
	explicit
	Stream_out(std::string &&name);

	void name(std::string &&name);
	std::string_view name();

	/// var length encoding
	void put_uint(u64 v);
	/// puts as is. e.g. always 8 bytes
	void put_uint64(u64 v);
	void pump(u8 *from, u64 size) override;
	void finish() override;
private:
	std::string name_;
	std::vector<u8> buff_;
};

inline
void Stream_in::name(std::string &&name)
{
	name_ = std::move(name);
}
inline
std::string_view Stream_in::name()
{
	return name_;
}
inline
std::string_view Stream_out::name()
{
	return name_;
}
inline
void Stream_out::name(std::string &&name)
{
	name_ = std::move(name);
}

void read_message(Buffer &message, Stream_in &in, Pipe_xxhash_in &cs_pipe);
template<class T>
T *get_message(Buffer &tmp, Stream_in &in, Pipe_xxhash_in &cs_pipe, google::protobuf::Arena &arena){
	T *msg = google::protobuf::Arena::CreateMessage<T>(&arena);
	read_message(tmp, in, cs_pipe);
	msg->ParseFromArray(tmp.raw(), tmp.size());
	return msg;
}

void put_message(google::protobuf::MessageLite &msg, Buffer &buff, Stream_out &out, Pipe_xxhash_out &cs_pipe);


}
