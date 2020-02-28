#include "restore.h"
#include "exception.h"
#include "catalogue.h"
#include "globals.h"
#include "platform.h"
#include "piping.h"
#include "checksumer.h"
#include "pump.h"

using namespace std;
using namespace fmt;
namespace fs = filesystem;

namespace archi{


void apply_attribs(fs::path &target, Filesystem_state::File &attr){
	if (!attr.acl.empty())
		set_acl(target, attr.acl);
	if (!attr.default_acl.empty())
		set_default_acl(target, attr.default_acl);
	if (attr.mod_time)
		fs::last_write_time(target, from_posix_time(*attr.mod_time));
	if (attr.unix_permissions)
		fs::permissions(target, static_cast<fs::perms>(*attr.unix_permissions));
}

void restore(Restore_settings &cfg)
{
	try{
		Buffer tmp;
		tmp.resize(128*1024);
		Catalogue cat(cfg.archive_path, cfg.password);
		auto num_ids = cat.num_states();
		if (num_ids == 0)
			throw Exception("the archive is empty.");
		auto state = cat.fs_state(cfg.from_ndx);
		auto all_files = state.files();
		vector<reference_wrapper<Filesystem_state::File>> files(all_files.begin(), all_files.end());
		auto mk_re_path = [&](fs::path &p){
			ASSERT(cfg.prefix.empty() or p.string().starts_with(cfg.prefix.string()));
			return cfg.to / p.lexically_relative(cfg.prefix.parent_path());
		};
		if (!cfg.prefix.empty()){
			erase_if(files, [&](auto &f){
				// this has to compare full path elements. not parts of them
				auto pref_it = cfg.prefix.begin();
				for (auto &pe : f.get().path){
					if (pref_it == cfg.prefix.end())
						return false;
					ASSERT(!pref_it->empty());
					if (pe != *pref_it)
						return true;
					++pref_it;
				}
				if (pref_it == cfg.prefix.end())
					return false;
				return true;
			});
		}
		for (Filesystem_state::File &file : files){ // restore dirs
			if (file.type != Filesystem_state::DIR)
				continue;
			auto re_path = mk_re_path(file.path);
			try{
				fs::create_directories(re_path);
			}
			catch(std::exception &e){
				cfg.warning(format(tr_txt("Can't restore directory {0} to {1}: "), file.path, re_path), message(e));
			}
		}
		{ // restore non empty files
			auto refs_only = files | ranges::view::remove_if([](auto &a){return !a.get().content_ref.has_value();});
			vector<reference_wrapper<Filesystem_state::File>> sorted_by_refs( refs_only.begin(), refs_only.end() );
			ranges::action::sort(sorted_by_refs, [](auto a, auto b){
				return a.get().content_ref.value() < b.get().content_ref.value();
			});
			File_source in;
			Stream_in sin;
			Filtrator_in filters;
			decltype(File_content_ref::fname) fname;
			decltype(File_content_ref::from)  num_pumped;
			Checksumer cs;
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
						fname = ref.fname;
						filters = Filtrator_in(ref.filters);
						sin << filters << in;
						cs.set_for(ref.csum);
					}
					pump(sin, ref.from, nullptr, ref.fname, tmp, num_pumped);
					File_sink out(re_path);
					Stream_out sout;
					cs.reset();
					sout >> cs.pipe() >> out;
					pump(sin, ref.to, &sout, ref.fname, tmp, num_pumped);
					if (ref.csum != cs.checksum())
						cfg.warning( fmt::format(tr_txt("Control sums do not match for {0}"), re_path), "" );
				}
				catch(std::exception &e){
					/* TRANSLATORS: This is about path from and to  */
					cfg.warning(format(tr_txt("Can't restore {0} to {1}: "), file.path, re_path), message(e));
				}
			}
		}
		for (Filesystem_state::File &file : files){ // restore links and empty files
			if (file.type == Filesystem_state::DIR)
				continue;
			auto re_path = mk_re_path(file.path);
			try{
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
				cfg.warning(format(tr_txt("Can't restore {0} to {1}: "), file.path, re_path), message(e));
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
				cfg.warning(format(tr_txt("Can't restore attributes for {0}: "), re_path), message(e));
			}
		}
	}
	catch(std::exception &e){
		string msg;
		if (cfg.name.empty())
			/* TRANSLATORS: This is about path from and to  */
			cfg.warning(format(tr_txt("Error while restoring from {0} to {1}"), cfg.archive_path, cfg.to), message(e));
		else
			/* TRANSLATORS: First argument is name, second - path*/
			cfg.warning(format(tr_txt("Error while restoring from {0} to {1}"), cfg.name, cfg.to), message(e));
	}
}

}
