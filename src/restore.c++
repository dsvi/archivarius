#include "restore.h"
#include "exception.h"
#include "catalogue.h"
#include "globals.h"
#include "platform.h"

using namespace std;
namespace fs = filesystem;

void pump_to(File_source &in, ui64 to, File_sink *out, string_view fname, Buffer &tmp, ui64 &num_pumped )
{
	while (num_pumped < to){
		auto num_left = to - num_pumped;
		auto to_pump = num_left > tmp.size() ? tmp.size() : num_left;
		auto ret = in.pump(tmp.raw(), to_pump);
		if (ret.eof && ret.pumped_size != to_pump)
			throw Exception( "Truncated content file %1%" ) << fname;
		if (out)
			out->pump(tmp.raw(), to_pump);
		num_pumped += to_pump;
	}
	ASSERT(num_pumped == to);
}

void apply_attribs(fs::path &target, Filesystem_state::File &attr){
	if (!attr.acl.empty())
		set_acl(target, attr.acl);
	if (!attr.default_acl.empty())
		set_default_acl(target, attr.default_acl);
	fs::last_write_time(target, from_posix_time(attr.mod_time));
	fs::permissions(target, static_cast<fs::perms>(attr.unix_permissions));
}

void restore(Restore_settings &cfg)
{
	try{
		Buffer tmp;
		tmp.resize(10'000'000);
		Catalogue catalog(cfg.from);
		auto state = catalog.fs_state(cfg.from_ndx);
		for (auto &file : state.files()){ // restore dirs
			if (file.type != Filesystem_state::DIR)
				continue;
			auto re_path = cfg.to / file.path;
			try{
				fs::create_directories(re_path);
				apply_attribs(re_path, file);
			}
			catch(std::exception &e){
				string msg = str(boost::format(tr_text("Can't restore directory %1% to %2%:\n")) % file.path % re_path);
				msg += message(e);
				cfg.warning(move(msg));
			}
		}
		{ // restore non empty files
			map<File_content_ref&, Filesystem_state::File&> sorted_by_refs;
			for (auto &file : state.files()){
				if (file.content_ref)
					sorted_by_refs[file.content_ref.value()] = file;
			}
			File_source in;
			decltype(File_content_ref::fname) fname;
			decltype(File_content_ref::from)  num_pumped;
			for (auto &p : sorted_by_refs){
				auto &ref = p.first;
				auto &file = p.second;
				auto re_path = cfg.to / file.path;
				try {
					if (fname != ref.fname){
						in = move(cfg.from / ref.fname);
						num_pumped = 0;
					}
					pump_to(in, ref.from, nullptr, ref.fname, tmp, num_pumped);
					File_sink out(re_path);
					pump_to(in, ref.to, &out, ref.fname, tmp, num_pumped);
					apply_attribs(re_path, file);
				}
				catch(std::exception &e){
					string msg = str(boost::format(tr_text("Can't restore %1% to %2%:\n")) % file.path % re_path );
					msg += message(e);
					cfg.warning(move(msg));
				}
			}
		}
		for (auto &file : state.files()){ // restore links and empty files
			if (file.type == Filesystem_state::DIR)
				continue;
			auto re_path = cfg.to / file.path;
			try{
				if (file.type == Filesystem_state::FILE){
					if (file.content_ref)
						continue;
					File_sink out(re_path);
				}
				else if (file.type == Filesystem_state::SYMLINK){
					std::filesystem::create_symlink(file.symlink_target, re_path);
				}
				apply_attribs(re_path, file);
			}
			catch(std::exception &e){
				string msg = str(boost::format(tr_text("Can't restore %1% to %2%:\n")) % file.path % re_path);
				msg += message(e);
				cfg.warning(move(msg));
			}
		}
	}
	catch(std::exception &e){
		string msg = str(boost::format(tr_text("Error while restoring %1% to %2%:\n")) % cfg.name % cfg.to);
		msg += message(e);
		cfg.warning(move(msg));
	}
}
