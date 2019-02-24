#pragma once
#include "piping.h"
#include "buffer.h"
#include <zstd.h>

class Pipe_zstd_out: public Pipe_out{
public:
	explicit
	Pipe_zstd_out(int compression_level);
private:
	virtual
	void pump(u8 *from, u64 size) override;
	virtual
	void finish() override;

	typedef std::unique_ptr<ZSTD_CStream, decltype(&ZSTD_freeCStream)> Zc_stream_ptr;

	Buffer out_buffer_;
	Zc_stream_ptr zstream_;
	bool i_pumped_ = false;
};


class Pipe_zstd_in: public Pipe_in{
public:
	Pipe_zstd_in();
private:
	typedef std::unique_ptr<ZSTD_DStream, decltype(&ZSTD_freeDStream)> Zd_stream_ptr;

	virtual
	Pump_result pump(u8 *to, u64 size) override;
	Buffer buffer_;
	ZSTD_inBuffer zin_;
	Zd_stream_ptr zstream_;
};
