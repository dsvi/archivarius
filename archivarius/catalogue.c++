#include "precomp.h"
#include "catalogue.h"
#include "exception.h"
#include "piping_xxhash.h"
#include "piping_zstd.h"
#include "stream.h"
#include "buffer.h"

#include "format.pb.h"

static const uint current_version = 0;

using namespace std;
namespace fs = std::filesystem;

static const char * cat_filename = "catalog";

namespace archi{


void add_filters(proto::Filters *pf, Filters_in &f){
	if (f.cmp_in)
		pf->mutable_zstd_compression();
	if (f.enc_chapo_in){
		auto enc = pf->mutable_chapoly_encryption();
		enc->set_iv(f.enc_chapo_in->iv(), f.enc_chapo_in->iv_size());
		enc->set_key(f.enc_chapo_in->key(), f.enc_chapo_in->key_size());
	}
	if (f.enc_chacha_in){
		auto enc = pf->mutable_chacha_encryption();
		enc->set_iv(f.enc_chacha_in->iv(), f.enc_chacha_in->iv_size());
		enc->set_key(f.enc_chacha_in->key(), f.enc_chacha_in->key_size());
	}
}

template<class PB_ENC_FILTER>
void fill_enc_params(PB_ENC_FILTER &fm, Encryption_params &ep){
	if (fm.key().size() != ep.key_size())
		throw Exception("Wrong encryption key size. Likely corrupt file.");
	if (fm.iv().size() != ep.iv_size())
		throw Exception("Wrong encryption IV size. Likely corrupt file.");
	ep.key(fm.key());
	ep.iv(fm.iv());
}

Filters_in get_filters(const proto::Filters &pf){
	Filters_in ret;
	if (pf.has_zstd_compression())
		ret.cmp_in.emplace();
	if (pf.has_chapoly_encryption()){
		auto &enc = ret.enc_chapo_in.emplace();
		auto penc = pf.chapoly_encryption();
		fill_enc_params(penc, enc);
	}
	if (pf.has_chacha_encryption()){
		auto &enc = ret.enc_chacha_in.emplace();
		auto penc = pf.chacha_encryption();
		fill_enc_params(penc, enc);
	}
	return ret;
}

Catalogue::Catalogue(std::filesystem::path &arc_path, std::string_view key)
{
	fs::create_directories(arc_path);
	cat_file_ = arc_path / cat_filename;
	if (!fs::exists(cat_file_))
		File_sink f(cat_file_);
	file_lock_ = lock(cat_file_);
	if (fs::file_size(cat_file_) == 0){
		clean_up();
		if (!key.empty()){
			enc_.emplace();
			using namespace chrono;
			//TODO: change to C++2a utc_clock
			auto t = duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
			std::vector<u8> iv(&t, &t + sizeof (t));
			iv.resize(enc_->iv_size());
			enc_->iv(iv);
			enc_->set_password(key);
		}
		return;
	}

	try {
		File_source src(cat_file_);
		Pipe_xxhash_in cs_pipe;
		Stream_in in(cat_file_);

		in << cs_pipe << src;

		if (auto version = in.get_uint(); version > current_version)
			throw Exception("Unsupported file version {0}. Max supported is {1}")(version, current_version);
		Buffer buf;
		auto header = get_message<proto::Catalog_header>(buf, in, cs_pipe);
		Filters_in filters;
		if (header.has_filters()){
			auto &f = header.filters();
			if (f.has_chapoly_encryption()){
				Chapoly &ein = filters.enc_chapo_in.emplace();
				auto &iv = f.chapoly_encryption().iv();
				if (iv.size() != ein.iv_size())
					throw Exception("Wrong encryption IV size. Likely corrupt file.");
				ein.iv(iv);
				ein.set_password(key);
				enc_ = ein;
				enc_->inc_iv();
			}
			if (f.has_zstd_compression())
				filters.cmp_in.emplace();
		}
		if (!enc_ and !key.empty())
			throw Exception("Archive was not encrypted before. You have to recreate it.");
		Filtrator_in fltr(filters);
		in << cs_pipe << fltr << src;

		auto catalog = get_message<proto::Catalogue>(buf, in, cs_pipe);
		// TODO: add more checks?
		for (auto &file: catalog.state_files()){
			Fs_state_file state;
			state.name = file.name();
			state.time_created = file.time_created();
			ASSERT(state.time_created);
			if (file.has_filters())
				state.filters = get_filters(file.filters());
			fs_state_files_.push_back(move(state));
		}

		for (auto &file: catalog.content_files()){
			File_content_ref ref;
			if (file.has_filters())
				ref.filters = get_filters(file.filters());
			ref.fname = file.name();
			for (auto &r : file.refs()){
				ref.from = r.from();
				ref.to   = r.to();
				ref.ref_count_ = r.ref_count();
				ref.space_taken = r.space_taken();
				ASSERT(ref.space_taken);
				if (r.has_xxhash())
					ref.csum.emplace<Xx_hash>(r.xxhash());
				if (r.has_blake2b()){
					auto &pb2b = r.blake2b();
					Blake2b_hash &b2b = ref.csum.emplace<Blake2b_hash>();
					if (pb2b.size() != sizeof(b2b))
						throw Exception("Wrong blake2b size. Likely corrupt file.");
					copy_n(pb2b.begin(), sizeof(b2b), b2b.begin());
				}
				content_refs_.insert(ref);
			}
		}
		clean_up();
	}
	catch (...){
		throw_with_nested( Exception("Can't read {0}")(cat_file_) );
	}
}

void Catalogue::password(string_view key)
{
	if (!enc_)
		throw Exception("Archive was not encrypted before. You have to recreate it.");
	if (key.empty())
		enc_.reset();
	enc_->set_password(key);
}

Filesystem_state Catalogue::fs_state(size_t ndx)
{
	if (ndx >= fs_state_files_.size())
		throw Exception("State #{0} doesn't exist")(ndx);
	auto &state_desc = fs_state_files_[ndx];
	return Filesystem_state(
	  cat_file_.parent_path(),
	  state_desc.name,
	  state_desc.time_created,
	  state_desc.filters,
		[this](File_content_ref &r) -> File_content_ref { return map_ref(r); });
}

Filesystem_state Catalogue::latest_fs_state()
{
	if (fs_state_files_.empty()){
		return empty_fs_state();
	}
	auto &state_desc = fs_state_files_.back();
	return Filesystem_state(
	  cat_file_.parent_path(),
	  state_desc.name,
	  state_desc.time_created,
	  state_desc.filters,
		[this](File_content_ref &r) -> File_content_ref { return map_ref(r); });
}

Filesystem_state Catalogue::empty_fs_state()
{
	Filters_out f;
	f.cmp_out = {14};
	if (enc_)
		f.enc_chapo_out.emplace().randomize();
	return Filesystem_state(cat_file_.parent_path(), f);
}

void Catalogue::add_fs_state(Filesystem_state &fs)
{
	Fs_state_file state_file;
	state_file.name = fs.file_name();
	state_file.time_created = fs.time_created();
	state_file.filters = fs.filters();
	fs_state_files_.insert(fs_state_files_.begin(), state_file);

	for (auto &file : fs.files()){
		if (!file.content_ref.has_value())
			continue;
		auto [it, was_inserted] = content_refs_.insert(file.content_ref.value());
		ASSERT( (was_inserted && it->ref_count_ == 0) || !was_inserted);
		File_content_ref &ref = const_cast<File_content_ref&>(*it);
		ref.ref_count_++;
	}
}

void Catalogue::remove_fs_state(Filesystem_state &fs)
{
	auto end = remove_if(fs_state_files_.begin(), fs_state_files_.end(), [&](auto &a){
		return a.name == fs.file_name();
	});
	if (end != --fs_state_files_.end())
		throw_inconsistent(__LINE__);
	fs_state_files_.pop_back();
	for (auto &file : fs.files()){
		if (!file.content_ref.has_value())
			continue;
		auto it = content_refs_.find(file.content_ref.value());
		ASSERT(it != content_refs_.end());
		if (it == content_refs_.end())
			throw_inconsistent(__LINE__);
		auto &ref = const_cast<File_content_ref&>(*it);
		if (--ref.ref_count_ == 0)
			content_refs_.erase(file.content_ref.value());
	}
}

void Catalogue::commit()
{
	try {
		auto new_file = cat_file_;
		new_file += ".tmp";
		File_sink dst(new_file);
		Stream_out out(new_file);
		Pipe_xxhash_out cs_pipe;
		out >> cs_pipe >> dst;

		out.put_uint(current_version);
		Buffer buf;
		{
			proto::Catalog_header hdr;
			auto f = hdr.mutable_filters();
			f->mutable_zstd_compression();
			if (enc_){
				auto enc = f->mutable_chapoly_encryption();
				enc->set_iv(enc_->iv(), enc_->iv_size());
			}
			put_message(hdr, buf, out, cs_pipe);
		}
		Filtrator_out filtr;
		filtr.compression({22});
		if (enc_)
			filtr.encryption(*enc_);
		out >> cs_pipe >> filtr >> dst;

		proto::Catalogue cat_msg;
		for (auto &fsf : fs_state_files_){
			auto sfile = cat_msg.add_state_files();
			sfile->set_name(fsf.name);
			sfile->set_time_created(fsf.time_created);
			if (fsf.filters){
				auto f = sfile->mutable_filters();
				add_filters(f, fsf.filters);
			}
		}
		string_view fn;
		proto::Content_file *cfile;
		for (auto &rc : content_refs_){
			auto &r = const_cast<File_content_ref&>(rc);
			if (fn != r.fname){
				fn = r.fname;
				cfile = cat_msg.add_content_files();
				cfile->set_name(r.fname);
				if (r.filters){
					auto f = cfile->mutable_filters();
					add_filters(f, r.filters);
				}
			}
			auto ref = cfile->add_refs();
			ref->set_from(r.from);
			ref->set_to(r.to);
			ASSERT(r.space_taken);
			ref->set_space_taken(r.space_taken);
			ASSERT(r.ref_count_);
			ref->set_ref_count(r.ref_count_);
			if (auto h = get_if<Xx_hash>(&r.csum))
				ref->set_xxhash(*h);
			if (auto h = get_if<Blake2b_hash>(&r.csum))
				ref->set_blake2b(h, sizeof(*h));
		}
		put_message(cat_msg, buf, out, cs_pipe);
		out.finish();
		#ifdef COMPRESS_STAT
		if (cat_msg.ByteSizeLong())
			fmt::print("Catalog compressed to {}% of original size\n", dst.bytes_written() *100/cat_msg.ByteSizeLong());
		#endif
		fs_sync();
		fs::rename(new_file, cat_file_);
		fs_sync();
	}
	catch (...){
		throw_with_nested( Exception( "Can't save {0}" )(cat_file_) );
	}
	clean_up();
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
		if (file_name.c_str()[0] == '.')
			continue;
		if (used.find(file_name) == used.end()){
			error_code ec;
			fs::remove(path_name, ec);
			ASSERT(!ec);
		}
	}
}

void Catalogue::throw_inconsistent(uint line)
{
	throw Exception("Archive is in inconsistent state, better recreate: {0}\ncode: {1}")(cat_file_.parent_path(), line);
}

File_content_ref Catalogue::map_ref(File_content_ref &r)
{
	auto it = content_refs_.find(r);
	ASSERT(it != content_refs_.end());
	if (it == content_refs_.end())
		throw_inconsistent(__LINE__);
	return const_cast<File_content_ref&>(*it);
}


}
