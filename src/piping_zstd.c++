#include "piping_zstd.h"
#include "exception.h"

static
void check_error(size_t code)
{
	if (ZSTD_isError(code))
		throw Exception("zstd de/compressor error: {0}")(ZSTD_getErrorName(code));
}


Pipe_zstd_out::Pipe_zstd_out(int compression_level) : zstream_(ZSTD_createCStream(), ZSTD_freeCStream)
{
	if (!zstream_)
		throw Exception("Can't initialize zstd compressor");
	auto err = ZSTD_initCStream(zstream_.get(), compression_level);
	check_error(err);
}

void Pipe_zstd_out::pump(u8 *from, u64 size)
{
	ZSTD_inBuffer zin;
	ZSTD_outBuffer zout;
	zin.pos = 0;
	zin.src = from;
	zin.size = size;
	if (size)
		i_pumped_ = true;
	out_buffer_.resize(size);
	zout.dst = out_buffer_.raw();
	zout.size = out_buffer_.size();
	do {
		zout.pos = 0;
		auto err = ZSTD_compressStream(zstream_.get(), &zout, &zin);
		check_error(err);
		pump_next(out_buffer_.raw(), zout.pos);
	} while(zin.pos != zin.size);
}

void Pipe_zstd_out::finish()
{
	if (!i_pumped_){
		finish_next();
		return;
	}
	ZSTD_outBuffer zout;
	zout.dst = out_buffer_.raw();
	zout.size = out_buffer_.size();
	size_t err;
	do{
		zout.pos = 0;
		err = ZSTD_endStream(zstream_.get(), &zout);
		check_error(err);
		pump_next(out_buffer_.raw(), zout.pos);
	} while(err != 0);
	finish_next();
}


Pipe_zstd_in::Pipe_zstd_in() : zstream_(ZSTD_createDStream(), ZSTD_freeDStream)
{
	if (!zstream_)
		throw Exception("Can't initialize zstd decompressor");
	auto err = ZSTD_initDStream(zstream_.get());
	check_error(err);
}

Source::Pump_result Pipe_zstd_in::pump(u8 *to, u64 size)
{
	ZSTD_outBuffer zout;
	zout.dst = to;
	zout.pos = 0;
	zout.size = size;
	if (buffer_.size() == 0){
		buffer_.resize(10*1024*1024);
		zin_.src = buffer_.raw();
		zin_.size = buffer_.size();
		zin_.pos = zin_.size;
	}
	Source::Pump_result res;
	res.eof = false;
	while (true) {
		if (zin_.pos == zin_.size){
			res = pump_next(buffer_.raw(), buffer_.size());
			zin_.pos = 0;
			zin_.size = res.pumped_size;
		}
		auto err = ZSTD_decompressStream(zstream_.get(), &zout, &zin_);
		check_error(err);
		res.pumped_size = zout.pos;
		if (zout.pos == zout.size){
			res.eof = false; // might be leftovers in the zstream.
			return res;
		}
		else if (res.eof){
			ASSERT(zin_.pos == zin_.size);
			return res;
		}
	}
}
