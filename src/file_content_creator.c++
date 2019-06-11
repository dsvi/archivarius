#include "file_content_creator.h"
#include "globals.h"


using namespace std;
namespace fs = std::filesystem;

File_content_creator::File_content_creator(const std::filesystem::path &arc_path)
{
	arc_path_ = arc_path;
	buff_.resize(128*1024);
}

void File_content_creator::enable_compression(Zstd_out &p)
{
	ASSERT(!file_sink_);
	filters_.compression(p);
}

void File_content_creator::enable_encryption()
{
	ASSERT(!file_sink_);
	enc_.emplace();
	cs_.set_for(Blake2b_hash());
}

File_content_ref File_content_creator::add(const std::filesystem::path &file_name)
{
	try {
		if (!file_sink_ || file_sink_.bytes_written() > min_file_size_ ){
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
		auto bytes_actually_wirtten = file_sink_.bytes_written();
		Source::Pump_result res;
		do{
			res = in_.pump(buff_.raw(), buff_.size());
			out_.pump(buff_.raw(), res.pumped_size);
			bytes_pumped_ += res.pumped_size;
		}while (!res.eof);
		ref.to = bytes_pumped_;
		ref.csum = cs_.checksum();
		// TODO: check if this shit actually works well
		filters_.flush_der_kompressor();
		ref.space_taken = file_sink_.bytes_written() - bytes_actually_wirtten;
		fmt::print("ref.space_taken {}", ref.space_taken);
		comp_ratio_.original += ref.to - ref.from;
		comp_ratio_.compressed += ref.space_taken;
		if (ref.space_taken == 0)
			ref.space_taken = 1;
		if (file_sink_.bytes_written() > min_file_size_)
			out_.finish();
		return ref;
	} catch (...) {
		file_sink_.reset();
		throw;
	}
}

void File_content_creator::finish()
{
	out_.finish();
}

void File_content_creator::create_file()
{
	out_.finish();
	fs::path file = arc_path_;
	fname_ = make_unique_filename(arc_path_, "c");
	file /= fname_;
	file_sink_ = file;
	if (enc_){
		enc_->randomize();
		filters_.encryption(*enc_);
	}
	out_ >> cs_.pipe() >> filters_ >> file_sink_;
	out_.name(file);
}
