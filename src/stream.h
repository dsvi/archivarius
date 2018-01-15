#pragma once
#include "precomp.h"
#include "piping.h"
#include "piping_xxhash.h"
#include "buffer.h"

class Stream_in : public Pipe_in
{
public:
	Stream_in();
	Stream_in(std::string &&name);

	void name(std::string &&name);
	std::string_view name();

	ui64 get_uint(); // ... or die trying
	Source::Pump_result pump(ui8 *to, ui64 size);
private:
	std::string name_;
	std::vector<ui8> buff_;
};

class Stream_out : public Pipe_out
{
public:
	Stream_out();
	Stream_out(std::string &&name);

	void name(std::string &&name);
	std::string_view name();

	void put_uint(ui64 v);
	void pump(ui8 *from, ui64 size);
private:
	std::string name_;
	std::vector<ui8> buff_;
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
T get_message(Buffer &tmp, Stream_in &in, Pipe_xxhash_in &cs_pipe){
	T msg;
	read_message(tmp, in, cs_pipe);
	msg.ParseFromArray(tmp.raw(), tmp.size());
	return msg;
}

void put_message(google::protobuf::MessageLite &msg, Buffer &buff, Stream_out &out, Pipe_xxhash_out &cs_pipe);
