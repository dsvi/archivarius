#include "piping.h"
#include "exception.h"
#include "globals.h"

using namespace std;
namespace fs = std::filesystem;

namespace archi{


static
void check_error(){
	if (!errno)
		return;
	auto err = errno;
	errno = 0;
	throw std::runtime_error(strerror(err));
}


File_source::File_source() :  file_(nullptr, checked_fclose)
{

}

File_source::File_source(const std::filesystem::path &path) : file_(fopen(path.c_str(),"rb"), checked_fclose)
{
	try{
		if (!file_)
			check_error();
	}
	catch(...){
		throw_with_nested( Exception("Couldn't open file {0} for reading")(path.native()) );
	}
}

Source::Pump_result File_source::pump(u8 *to, u64 size)
{
	Source::Pump_result res;
	res.pumped_size = fread( to, 1, size , file_.get());
	if (ferror(file_.get()))
		check_error();
	res.eof = feof(file_.get());
	return res;
}

File_sink::File_sink() : file_(nullptr, checked_fclose)
{

}

File_sink::File_sink(const std::filesystem::path &path): file_(fopen(path.c_str(),"wb"), checked_fclose)
{
	try{
		if (!file_)
			check_error();
	}
	catch(...){
		throw_with_nested( Exception("Couldn't open file {0} for writing")(path.native()) );
	}
}

void File_sink::reset()
{
	bytes_written_ = 0;
	file_.reset(nullptr);
}

void File_sink::pump(u8 *to, u64 size)
{
	bytes_written_ += fwrite(to, 1, size , file_.get());
	if (ferror(file_.get()))
		check_error();
}

void File_sink::finish()
{
	file_.reset(nullptr);
}

void Pipe_out::finish_next()
{
	if (next_)
		next_->finish();
}

void Pipe_out::finish()
{
	finish_next();
}

void Pipe_out::pump_next(u8 *from, u64 size)
{
	if (next_)
		next_->pump(from, size);
}

Source::~Source()
{

}

Sink::~Sink()
{

}

Source::Pump_result Pipe_in::pump_next(u8 *to, u64 size)
{
	return next_->pump(to, size);
}

void checked_fclose(FILE *f)
{
	auto err = fclose(f);
	if (uncaught_exceptions())
		return;
	if (err)
		check_error();
}

}
