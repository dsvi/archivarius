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

void File_content_creator::enable_compression(Zstd_params &p)
{
	ASSERT(!file_);
	zstd_ = p;
}

File_content_ref File_content_creator::add(const std::filesystem::path &file_name)
{
	if (!file_ || file_.bytes_written() > min_file_size_ ){
		create_file();
		bytes_pumped_ = 0;
	}
	File_source src(file_name);
	in_.name(file_name);
	in_.source(&src);
	cs_.reset();
	File_content_ref ref;
	ref.fname = fname_;
	ref.from = bytes_pumped_;
	auto bytes_actually_wirtten = file_.bytes_written();
	Source::Pump_result res;
	do{
		res = in_.pump(buff_.raw(), buff_.size());
		out_.pump(buff_.raw(), res.pumped_size);
		bytes_pumped_ += res.pumped_size;
	}while (!res.eof);
	ref.to = bytes_pumped_;
	out_.put_uint64(cs_.digest());
	bytes_pumped_ += sizeof(cs_.digest());
	if (file_.bytes_written() > min_file_size_)
		out_.finish();
	ref.space_taken = file_.bytes_written() - bytes_actually_wirtten;
	if (ref.space_taken == 0)
		ref.space_taken = 1;
	return ref;
}

void File_content_creator::finish()
{
	out_.finish();
}

void File_content_creator::create_file()
{
	out_.finish();
	fs::path file;
	int count = 0;
	do {
		file = arc_path_;
		fname_ = prefix + current_time_to_filename();
		if (count++)
			fname_ += string("#") + to_string(count -2);
		file /= fname_;
	}while (fs::exists(file));
	file_ = file;
	cs_.sink(&file_);
	out_.sink(&cs_);
	out_.name(file);
}
