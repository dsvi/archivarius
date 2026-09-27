#include "restore.h"
#include "exception.h"
#include "catalogue.h"
#include "globals.h"
#include "time.h"
#include "piping_csum.h"
#include "platform.h"
#include "piping.h"
#include "checksumer.h"
#include "pump.h"
#include <progress_bar.h>

using namespace std;
using namespace coformat;
namespace fs = filesystem;

namespace archi{


void apply_attribs(fs::path &target, Filesystem_state::File &attr){
	if (!attr.acl.empty())
		set_acl(target, attr.acl);
	if (!attr.default_acl.empty())
		set_default_acl(target, attr.default_acl);
	if (attr.mod_time)
		fs::last_write_time(target, attr.mod_time->to_file_time());
	if (attr.unix_permissions)
		fs::permissions(target, static_cast<fs::perms>(*attr.unix_permissions));
}

void Restore_action::restore()
{
	try{
		Pump pump;
		Catalogue cat(archive_path, password, false);
		auto num_ids = cat.num_states();
		if (num_ids == 0)
			throw Exception("the archive is empty.");
		auto state = cat.fs_state(from_ndx);
		auto all_files = state.files();
		vector<reference_wrapper<Filesystem_state::File>> files(all_files.begin(), all_files.end());
		auto mk_re_path = [&](fs::path &p){
			ASSERT(prefix.empty() or p.string().starts_with(prefix.string()));
			return to / p.lexically_relative(prefix.parent_path());
		};
		// A directory is in the archive only if it was archived itself, while a file from it can be.
		// (see 'include' taking priority over 'exclude'). So make sure there is somewhere to put it.
		auto ensure_parent = [&](fs::path &p){
			auto parent = p.parent_path();
			if (!parent.empty() and not fs::is_directory(parent))
				fs::create_directories(parent);
		};
		if (!prefix.empty()){
			erase_if(files, [&](auto &f){
				// this has to compare full path elements. not parts of them
				auto pref_it = prefix.begin();
                for (const auto &pe : f.get().path){
					if (pref_it == prefix.end())
						return false;
					ASSERT(!pref_it->empty());
					if (pe != *pref_it)
						return true;
					++pref_it;
				}
				if (pref_it == prefix.end())
					return false;
				return true;
			});
			if (files.empty())
				warning(tr_txt("The archive does not contain anything with the given prefix"),"");
		}
		for (Filesystem_state::File &file : files){ // restore dirs
			if (file.type != Filesystem_state::DIR)
				continue;
			auto re_path = mk_re_path(file.path);
			try{
				fs::create_directories(re_path);
			}
			catch(std::exception &e){
				warning(cformat(tr_txt("Can't restore directory {0} to {1}: "), file.path, re_path), message(e));
			}
		}
		{ // restore non empty files
			vector<reference_wrapper<Filesystem_state::File>> sorted_by_refs =
				files | views::filter([](auto &a){return a.get().content_ref.has_value();}) | ranges::to<vector>();
			ranges::sort(sorted_by_refs, [](auto a, auto b){
				return a.get().content_ref.value() < b.get().content_ref.value();
			});
			u64 total_bytes = 0; // total bytes to be written
			for (auto &fr : sorted_by_refs)
				total_bytes += fr.get().content_ref->to - fr.get().content_ref->from;
			progress->min = 0;
			progress->max = total_bytes;
			progress->update(0);
			const u64 pump_chunk = max<u64>(100, total_bytes / 100); // Progress_bar can't indicate finer than 1%
			u64 bytes_written = 0;
			File_source in;
			Stream_in sin;
			Filtrator_in filters;
			decltype(File_content_ref::fname) fname;
			decltype(File_content_ref::from)  num_pumped;
			Pipe_csum_out cs_out;
			for (auto fr : sorted_by_refs){
				auto &file = fr.get();
				auto &ref = file.content_ref.value();
				auto re_path = mk_re_path(file.path);
				try {
					if (fname != ref.fname){
						auto content_path = cat.archive_path() / ref.fname;
						in = content_path;
						sin.name(content_path);
						num_pumped = 0;
						filters = Filtrator_in(ref.filters);
						sin << filters << in;
						cs_out.csumer_for(ref.csum);
						fname = ref.fname;
					}
					pump.pump(sin, ref.from, nullptr, ref.fname, num_pumped);
					ensure_parent(re_path);
					File_sink out(re_path);
					Stream_out sout;
					cs_out.csumer()->reset();
					sout >> cs_out >> out;
					while (num_pumped < ref.to){
						pump.pump(sin, min(ref.to, num_pumped + pump_chunk), &sout, ref.fname, num_pumped);
						progress->update(bytes_written + num_pumped - ref.from);
					}
					if (ref.csum != cs_out.csumer()->checksum())
						warning(cformat(tr_txt("Control sums do not match for {0}"), re_path), "" );
					sout.finish();
				}
				catch(std::exception &e){
					/* TRANSLATORS: This is about path from and to  */
					warning(cformat(tr_txt("Can't restore {0} to {1}: "), file.path, re_path), message(e));
				}
				bytes_written += ref.to - ref.from; // account for what would have been written if all went well
			}
		}
		for (Filesystem_state::File &file : files){ // restore links and empty files
			if (file.type == Filesystem_state::DIR)
				continue;
			auto re_path = mk_re_path(file.path);
			try{
				ensure_parent(re_path);
				if (file.type == Filesystem_state::FILE){
					if (file.content_ref)
						continue;
					File_sink out(re_path);
				}
				else if (file.type == Filesystem_state::SYMLINK){
					std::filesystem::create_symlink(file.symlink_target, re_path);
				}
			}
			catch(std::exception &e){
				/* TRANSLATORS: This is about path from and to  */
				warning(cformat(tr_txt("Can't restore {0} to {1}: "), file.path, re_path), message(e));
			}
		}
		sort(files.begin(), files.end(), [](auto a, auto b){
			return a.get().path > b.get().path;
		});
		for (Filesystem_state::File &file : files){// restore attributes
			auto re_path = mk_re_path(file.path);
			try{
				apply_attribs(re_path, file);
			}
			catch(std::exception &e){
				warning(cformat(tr_txt("Can't restore attributes for {0}: "), re_path), message(e));
			}
		}
	}
	catch(std::exception &e){
		string msg;
		if (name.empty())
			/* TRANSLATORS: This is about path from and to  */
			warning(cformat(tr_txt("Error while restoring from {0} to {1}"), archive_path, to), message(e));
		else
			/* TRANSLATORS: First argument is name, second - path*/
			warning(cformat(tr_txt("Error while restoring from {0} to {1}"), name, to), message(e));
	}
}

}
