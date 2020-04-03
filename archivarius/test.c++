#include <tuple>
#include "test.h"
#include "precomp.h"
#include "globals.h"
#include "exception.h"
#include "pump.h"
#include "catalogue.h"
#include "checksumer.h"

namespace archi{

using namespace std;
using namespace fmt;

void test(Test_settings &cfg)
{
	try{
		Buffer tmp;
		tmp.resize(128*1024);
		Catalogue cat(cfg.archive_path, cfg.password);
		typedef tuple<string, u64> Discovered_key;
		std::map<Discovered_key, u64> discovered_refs;
		cfg.progress_status(tr_txt("Checking versions."));
		for (size_t i = 0; i < cat.num_states(); i++){
			cfg.progress(i * 1000 / cat.num_states());
			auto fs = cat.fs_state(i);
			for (auto &f: fs.files()){
				if (f.content_ref.has_value())
					++discovered_refs[{f.content_ref->fname, f.content_ref->from}];
			}
		}
		cfg.progress_status(tr_txt("Checking references consistency."));
		for (auto &cf : cat.content_refs()){
			Discovered_key t = {cf.fname, cf.from};
			if (!discovered_refs.contains(t)){
				cfg.warning(tr_txt("A useless ref is still in catalog."), cf.fname +":"+ to_string(cf.from));
				continue;
			}
			auto &r = discovered_refs[t];
			r -= cf.ref_count_;
			if (r)
				cfg.warning(tr_txt("Factual ref count doesnt match with catalog."), cf.fname +":"+ to_string(cf.from));
			discovered_refs.erase(t);
		}
		if (!discovered_refs.empty())
			cfg.warning(tr_txt("Some refs are used but are not in catalog."), "");

		cfg.progress_status(tr_txt("Checking files content."));
		auto total_refs = cat.content_refs().size();
		uint progress = 10000;
		File_source in;
		Stream_in sin;
		Stream_out sout;
		Filtrator_in filters;
		Checksumer cs;
		decltype(File_content_ref::fname) fname;
		decltype(File_content_ref::from)  num_pumped;
		for (decltype(total_refs) i = 0; auto ref : cat.content_refs()){
			ASSERT(total_refs);
			uint p = i++ *1000 / total_refs;
			if (p != progress){
				cfg.progress(p);
				progress = p;
			}
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
				pump(sin, ref.from, nullptr, fname, tmp, num_pumped);
				cs.reset();
				sout >> cs.pipe();
				pump(sin, ref.to, &sout, fname, tmp, num_pumped);
				if (ref.csum != cs.checksum())
					cfg.warning( fmt::format(tr_txt("File {0} is broken."), fname), "Control sums do not match." );
			}
			catch(std::exception &e){
				/* TRANSLATORS: This is about path from and to  */
				cfg.warning(format(tr_txt("Problem with {0}"), ref.fname), message(e));
			}
		}
	}
	catch(std::exception &e){
		string msg;
		if (cfg.name.empty())
			/* TRANSLATORS: This is about path from and to  */
			cfg.warning(format(tr_txt("Error while testing {0}"), cfg.archive_path), message(e));
		else
			/* TRANSLATORS: First argument is name, second - path*/
			cfg.warning(format(tr_txt("Error while testing {0}"), cfg.name), message(e));
	}
}


}
