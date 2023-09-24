#include "filesystem_state.h"
#include "checksumer_xxhash.h"
#include "globals.h"
#include "exception.h"
#include "piping_csum.h"
#include "format.pb.h"
#include "stream.h"

using namespace std;
namespace fs = filesystem;

namespace archi{


Filesystem_state::Filesystem_state(const std::filesystem::path &arc_path, Filters_out &f)
{
	filename_ = make_unique_filename(arc_path, "s");
	time_created_ = to_posix_time(fs::file_time_type::clock::now());
	arc_path_ = arc_path;
	filtrator_.set_filters(f);
}

#pragma GCC diagnostic ignored "-Wreturn-type"
static Filesystem_state::File_type from_proto(proto::File_type ft){
	switch(ft){
	case proto::File_type::FILE:
		return Filesystem_state::FILE;
	case proto::File_type::DIR:
		return Filesystem_state::DIR;
	case proto::File_type::SYMLINK:
		return Filesystem_state::SYMLINK;
	default:
		ASSERT(0);
	}
}

Filesystem_state::Filesystem_state(
	const std::filesystem::path &arc_path,
	std::string_view name,
	Time time_created,
	Filters_in &f,
	std::function<File_content_ref(File_content_ref&)> ref_mapper)
{
	filename_ = name;
	arc_path_ = arc_path;
	time_created_ = time_created;

	auto fn = arc_path_ / file_name();
	Filtrator_in filtr(f);
	File_source file(fn);
	auto cs_tmp = make_unique<Checksumer_xxhash>();
	auto &cs = *cs_tmp.get();
	Pipe_csum_in cs_in(move(cs_tmp));

	Stream_in in(fn);
	in << cs_in << filtr << file;

	Buffer buf;
	google::protobuf::Arena arena;
	auto state = get_message<proto::Fs_state>(buf, in, cs, arena);
	for (auto &r : state->rec()){
		File f;
		f.path = r.pathname();
		f.type = from_proto(r.type());
		if (r.has_ref()){
			File_content_ref incomplete_ref;
			auto &ref = r.ref();
			incomplete_ref.fname = ref.content_fname();
			incomplete_ref.from = ref.from();
			f.content_ref = ref_mapper(incomplete_ref);
		}
		if (f.type == SYMLINK)
			f.symlink_target = r.symlink_target();
		else{
			if (r.has_modified_nanoseconds())
				f.mod_time = r.modified_nanoseconds();
			if (r.has_unix_permissions())
				f.unix_permissions = r.unix_permissions();
			if (r.has_posix_acl())
				f.acl = r.posix_acl();
			if (f.type == DIR and r.has_posix_default_acl())
				f.default_acl = r.posix_default_acl();
		}
		add(move(f));
	}
}

void Filesystem_state::add(Filesystem_state::File &&f)
{
	ASSERT(!f.path.empty());
	ASSERT(files_.find(f.path) == files_.end());
	files_[f.path] = move(f);
}

std::optional<File_content_ref> Filesystem_state::get_ref_if_exist(std::filesystem::path &archive_path, Time modified_time)
{
	auto it = files_.find(archive_path);
	if (it == files_.end())
		return optional<File_content_ref>();
	if (it->second.mod_time.value() != modified_time)
		return optional<File_content_ref>();
	return it->second.content_ref;
}

static proto::File_type to_proto(Filesystem_state::File_type ft){
	switch(ft){
	case Filesystem_state::FILE:
		return proto::File_type::FILE;
	case Filesystem_state::DIR:
		return proto::File_type::DIR;
	case Filesystem_state::SYMLINK:
		return proto::File_type::SYMLINK;
	default:
		ASSERT(0);
	}
}

void Filesystem_state::commit()
{
	auto fn = arc_path_ / file_name();
	if (fs::exists(fn))
		throw Exception("File {0} already exist")(fn.native());
	File_sink file(fn);
	auto cs_tmp = make_unique<Checksumer_xxhash>();
	auto &cs = *cs_tmp.get();
	Pipe_csum_out cs_out(move(cs_tmp));
	Stream_out out(fn);
	out >> cs_out >> filtrator_ >> file;

	google::protobuf::Arena arena;
	proto::Fs_state *state = google::protobuf::Arena::CreateMessage<proto::Fs_state>(&arena);
	for (auto &f : files()){
		auto rec = state->add_rec();
		rec->set_pathname(f.path);
		rec->set_type(to_proto(f.type));
		if (f.content_ref){
			auto ref = rec->mutable_ref();
			auto &fref = f.content_ref.value();
			ref->set_content_fname(fref.fname);
			ref->set_from(fref.from);
		}
		if (SYMLINK == f.type)
			rec->set_symlink_target(f.symlink_target);
		if (f.mod_time)
			rec->set_modified_nanoseconds(*f.mod_time);
		if (f.unix_permissions)
			rec->set_unix_permissions(*f.unix_permissions);
		if (!f.acl.empty())
			rec->set_posix_acl(f.acl);
		if (!f.default_acl.empty())
			rec->set_posix_default_acl(f.default_acl);
	}
	Buffer buf;
	put_message(*state, buf, out, cs);
	out.finish();
  #ifdef COMPRESS_STAT
	if (state->ByteSizeLong())
		fmt::print("Filesystem state compressed to {}% of original size\n", file.bytes_written() *100/state->ByteSizeLong());
  #endif
}


}
