#include "piping.h"
#include "exception.h"
#include "globals.h"

using namespace std;
namespace fs = std::filesystem;

static
void check_error(){
	if (!errno)
		return;
	auto err = errno;
	errno = 0;
	throw std::runtime_error(strerror(err));
}


File_source::File_source(const std::filesystem::path &path) : file_(fopen(path.c_str(),"rb"),fclose)
{
	try{
		check_error();
	}
	catch(...){
		throw_with_nested( Exception("Couldn't open file %1% for reading") << path.native() );
	}
}

Source::Pump_result File_source::pump(ui8 *to, ui64 size)
{
	Source::Pump_result res;
	res.pumped_size = fread( to, 1, size , file_.get());
	check_error();
	res.eof = feof(file_.get());
	return res;
}

File_sink::File_sink(const std::filesystem::path &path): file_(fopen(path.c_str(),"wb"), fclose)
{
	try{
		check_error();
	}
	catch(...){
		throw_with_nested( Exception("Couldn't open file %1% for writing") << path.native() );
	}
}

void File_sink::pump(ui8 *to, ui64 size)
{
	fwrite(to, 1, size , file_.get());
	bytes_written_ += size;
	check_error();
}

void File_sink::finish()
{
	file_.reset(nullptr);
}

void Pipe_out::finish()
{
	next_->finish();
}
