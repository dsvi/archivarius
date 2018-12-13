#include "file_content_creator.h"
#include "globals.h"


using namespace std;
namespace fs = std::filesystem;

std::string File_content_creator::prefix = "c";

File_content_creator::File_content_creator(const std::filesystem::path &arc_path)
{
	arc_path_ = arc_path;
	buff_.resize(50'000'000);
}

File_content_ref File_content_creator::add(const std::filesystem::path &file_name)
{
	if ((file_.bytes_written() > min_file_size_) || !file_){
		create_file();
		bytes_written_ = 0;
	}
	File_source src(file_name);
	in_.name(file_name);
	in_.source(&src);
	cs_.reset();
	File_content_ref ref;
	ref.fname = out_.name();
	ref.from = bytes_written_;
	auto actually_wirtten = file_.bytes_written();
	Source::Pump_result res;
	do{
		res = in_.pump(buff_.raw(), buff_.size());
		out_.pump(buff_.raw(), res.pumped_size);
		bytes_written_ += res.pumped_size;
	}while (!res.eof);
	ref.to = bytes_written_;
	out_.put_uint64(cs_.digest());
	bytes_written_ += sizeof(cs_.digest());
	if (file_.bytes_written() > min_file_size_)
		out_.finish();
	ref.compressed_size = file_.bytes_written() - actually_wirtten;
	if (ref.compressed_size == 0)
		ref.compressed_size = 1;
	return ref;
}

void File_content_creator::finish()
{
	out_.finish();
}

void File_content_creator::create_file()
{
	out_.finish();
	auto file = arc_path_;
	file /= prefix + current_time_to_filename();
	int count = 0;
	auto ofile = file;
	while (fs::exists(file)){
		file = ofile;
		file += string("#") + to_string(count++);
	}
	cs_.sink(&file_);
	out_.sink(&cs_);
	out_.name(file);
	file_ = file;
}
