#include "precomp.h"
#include "catalogue.h"
#include "exception.h"
#include "piping_xxhash.h"
#include "stream.h"
#include "buffer.h"
#include "globals.h"

#include "format.pb.h"

static const uint current_version = 0;

using namespace std;
namespace fs = std::filesystem;

const char * cat_filename = "catalog";

Catalogue::Catalogue(std::filesystem::path &arc_path)
{
	fs::create_directories(arc_path);
	cat_file_ = arc_path / cat_filename;
	if (!fs::exists(cat_file_)){
		{	File_sink f(cat_file_);	}
		file_lock_ = lock(cat_file_);
		return;
	}

	try {
		file_lock_ = lock(cat_file_);
		File_source src(cat_file_);
		Pipe_xxhash_in cs_pipe;
		cs_pipe.source(&src);
		Stream_in in(cat_file_);
		in.source(&cs_pipe);

		if (auto version = in.get_uint(); version > current_version)
			throw Exception("Unsupported file version %1%. Max supported is %2%") << version << current_version;

		Buffer buf;

		auto header = get_message<proto::Catalog_header>(buf, in, cs_pipe);
		// TODO: setup piping
	//	for (auto &f :header.filters()){
	//	}

		auto catalog = get_message<proto::Catalogue>(buf, in, cs_pipe);
		for (auto &file: catalog.used_files()){
			if (file.refs_size() == 0){
				Fs_state_file state;
				state.name = file.name();
				state.time_created = file.time_created();
				ASSERT(state.time_created);
				for (auto &filter: file.filters()){
					if (filter.has_aes_encryption())
						state.aes_encripted = true;
					if (filter.has_zstd_compression())
						state.zstd_compressed = true;
				}
				fs_state_files_.push_back(move(state));
			}
			else{
				File_content_ref ref;
				ref.fname = file.name();
				for (auto &r : file.refs()){
					ref.from = r.from();
					ref.to   = r.to();
					ref.ref_count_ = r.ref_count();
					ref.compressed_size = r.compressed_size();
					if (ref.compressed_size == 0)
						ref.compressed_size = ref.to - ref.from;
					ref.xxhash = r.xxhash();
					content_refs_.insert(ref);
				}
			}
		}
		clean_up();
	}
	catch (...){
		throw_with_nested( Exception("Can't read %1%") << cat_file_ );
	}
}

Filesystem_state Catalogue::fs_state(size_t ndx)
{
	if (ndx >= fs_state_files_.size())
		throw Exception("State #%1% doesn't exist") << ndx;
	auto &state_desc = fs_state_files_[ndx];
	return Filesystem_state(
	  cat_file_.parent_path(),
	  state_desc.name,
	  state_desc.time_created);
}

Filesystem_state Catalogue::latest_fs_state()
{
	if (fs_state_files_.empty())
		return Filesystem_state(cat_file_.parent_path());
	auto &state_desc = fs_state_files_.back();
	return Filesystem_state(
	  cat_file_.parent_path(),
	  state_desc.name,
	  state_desc.time_created);
}

void Catalogue::add_fs_state(Filesystem_state &fs)
{
	Fs_state_file state_file;
	state_file.name = fs.file_name();
	state_file.time_created = fs.time_created();
	fs_state_files_.push_back(state_file);

	for (auto &file : fs.files()){
		if (!file.content_ref.has_value())
			continue;
		auto [cit, was_inserted] = content_refs_.insert(file.content_ref.value());
		auto &ref = const_cast<File_content_ref&>(*cit);
		ref.ref_count_++;
	}
}

void Catalogue::remove_fs_state(Filesystem_state &fs)
{
	auto sz = fs_state_files_.size();
	remove_if(fs_state_files_.begin(), fs_state_files_.end(), [&](auto &a){
		return a.name == fs.file_name();
	});
	ASSERT(sz != fs_state_files_.size()); // trying to remove state which wasn't there
	if (sz == fs_state_files_.size())
		throw_inconsistent();
	for (auto &file : fs.files()){
		if (!file.content_ref.has_value())
			continue;
		auto cit = content_refs_.find(file.content_ref.value());
		ASSERT(cit != content_refs_.end());
		if (cit == content_refs_.end())
			throw_inconsistent();
		auto &ref = const_cast<File_content_ref&>(*cit);
		if (--ref.ref_count_ == 0)
			content_refs_.erase(file.content_ref.value());
	}
}

void Catalogue::commit()
{
	try {
		auto new_file = cat_file_;
		new_file.append(".tmp");
		File_sink dst(new_file);
		Pipe_xxhash_out cs_pipe;
		cs_pipe.sink(&dst);
		Stream_out out(new_file);
		out.sink(&cs_pipe);

		out.put_uint(current_version);
		proto::Catalog_header hdr;
		//TODO: filters

		Buffer buf;
		put_message(hdr, buf, out, cs_pipe);

		proto::Catalogue cat_msg;
		for (auto &fsf : fs_state_files_){
			auto desc = cat_msg.add_used_files();
			desc->set_name(fsf.name);
			desc->set_time_created(fsf.time_created);
			if (fsf.aes_encripted)
				desc->add_filters()->mutable_aes_encryption()->set_salt(""); //TODO: posolit
			if (fsf.zstd_compressed)
				desc->add_filters()->mutable_zstd_compression();
		}
		string_view fn;
		proto::File_desc *fd;
		for (auto &r : content_refs_){
			if (fn != r.fname){
				fd = cat_msg.add_used_files();
				fn = r.fname;
			}
			//TODO: filters
			auto ref = fd->add_refs();
			ref->set_from(r.from);
			ref->set_to(r.to);
			ref->set_xxhash(r.xxhash);
			ref->set_compressed_size(r.compressed_size);
			ref->set_ref_count(r.ref_count_);
		}
		put_message(cat_msg, buf, out, cs_pipe);
		out.finish();
		fs_sync();
		fs::rename(new_file, cat_file_);
		fs_sync();
		clean_up();
	}
	catch (...){
		throw_with_nested( Exception( "Can't save %1%" ) << cat_file_ );
	}
}

std::unordered_set<string> Catalogue::used_files()
{
	std::unordered_set<string> ret;
	ret.insert(cat_filename);
	for (auto &r : content_refs_)
		ret.insert(r.fname);
	for (auto &fs : fs_state_files_)
		ret.insert(fs.name);
	return ret;
}

void Catalogue::clean_up()
{
	auto used = used_files();
	auto dir = cat_file_.parent_path();
	for (auto &f : fs::directory_iterator(dir)){
		auto path_name = f.path();
		auto file_name = path_name.filename();
		if (used.find(file_name) == used.end())
			fs::remove(path_name);
	}
}

void Catalogue::throw_inconsistent()
{
	throw Exception("Archive is in inconsistent state, better recreate: %1%") << cat_file_.parent_path();
}


