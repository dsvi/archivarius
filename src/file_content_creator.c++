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

void File_content_creator::enable_compression(Zstd_out &p)
{
	ASSERT(!file_);
	filters_.compression(p);
}

void File_content_creator::enable_encryption()
{
	ASSERT(!file_);
	enc_params_.emplace();
}

File_content_ref File_content_creator::add(const std::filesystem::path &file_name)
{
	if (!file_ || file_.bytes_written() > min_file_size_ ){
		create_file();
		bytes_pumped_ = 0;
	}
	File_source src(file_name);
	in_.name(file_name);
	in_ << src;
	cs_.reset();
	File_content_ref ref;
	ref.filters = filters_.get_filters();
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
	ref.xxhash = cs_.digest();
	bytes_pumped_ += sizeof(cs_.digest());
	if (file_.bytes_written() > min_file_size_)
		out_.finish();
	// TODO: check if this shit actually works well
	filters_.flush_der_kompressor();
	ref.space_taken = file_.bytes_written() - bytes_actually_wirtten;
	if (ref.space_taken == 0)
		ref.space_taken = 1;
	comp_ratio_.original += ref.to - ref.from;
	comp_ratio_.compressed += ref.space_taken;
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
	if (enc_params_){
		enc_params_->randomize();
		filters_.encryption(*enc_params_);
	}
	out_ >> cs_ >> filters_ >> file_;
	out_.name(file);
}
