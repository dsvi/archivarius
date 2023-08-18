#pragma once
#include "checksumer_xxhash.h"
#include "precomp.h"
#include "piping.h"
#include "buffer.h"
#include "exception.h"

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

	// will be added to all the exceptions thrown from this class
	void error_tag(Exception::Tag t){
		error_tag_ = t;
	}
	// run io related functions in the context of current stream, so its exceptions will be caught,
	// and sane messages will be formed, and tags will be added
	void run(std::function<void()> &&f){
		try{
			f();
		}
		catch(std::exception&){
			throw_default_error();
		}
	}
private:
	Exception::Tag error_tag_;
	std::string name_;
	std::vector<u8> buff_{10};
	[[ noreturn ]]
	void throw_default_error();
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

void read_message(Buffer &message, Stream_in &in, Checksumer_xxhash &cs);
template<class T>
T *get_message(Buffer &tmp, Stream_in &in, Checksumer_xxhash &cs_pipe, google::protobuf::Arena &arena){
	T *msg = google::protobuf::Arena::CreateMessage<T>(&arena);
	read_message(tmp, in, cs_pipe);
	msg->ParseFromArray(tmp.raw(), tmp.size());
	return msg;
}

void put_message(google::protobuf::MessageLite &msg, Buffer &buff, Stream_out &out, Checksumer_xxhash &csr);


}
