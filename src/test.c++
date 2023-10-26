#include "piping_csum.h"
#include "test.h"
#include "precomp.h"
#include "globals.h"
#include "exception.h"
#include "pump.h"
#include "catalogue.h"
#include "checksumer.h"

namespace archi{

using namespace std;
using namespace coformat;

void Test_action::test()
{
	try{
		Buffer tmp;
		tmp.resize(128*1024);
		Catalogue cat(archive_path, password, false);
		typedef tuple<string, u64> Discovered_key;
		std::map<Discovered_key, u64> discovered_refs;
		progress_status(tr_txt("Checking versions."));
		for (size_t i = 0; i < cat.num_states(); i++){
			progress(i * 1000 / cat.num_states());
			auto fs = cat.fs_state(i);
			for (auto &f: fs.files()){
				if (f.content_ref.has_value())
					++discovered_refs[{f.content_ref->fname, f.content_ref->from}];
			}
		}
		progress_status(tr_txt("Checking references consistency."));
		for (auto &cf : cat.content_refs()){
			Discovered_key t = {cf.fname, cf.from};
			if (!discovered_refs.contains(t)){
				warning(tr_txt("A useless ref is still in catalog."), cf.fname +":"+ to_string(cf.from));
				continue;
			}
			auto &r = discovered_refs[t];
			r -= cf.ref_count_;
			if (r)
				warning(tr_txt("Factual ref count doesnt match with catalog."), cf.fname +":"+ to_string(cf.from));
			discovered_refs.erase(t);
		}
		if (!discovered_refs.empty())
			warning(tr_txt("Some refs are used but are not in catalog."), "");

		progress_status(tr_txt("Checking files content."));
		auto total_refs = cat.content_refs().size();
		uint reported_progress = numeric_limits<uint>::max();
		File_source in;
		Stream_in sin;
		Stream_out sout;
		Filtrator_in filters;
		Pipe_csum_out cs;
		decltype(File_content_ref::fname) fname;
		decltype(File_content_ref::from)  num_pumped;
		for (decltype(total_refs) i = 0; auto ref : cat.content_refs()){
			ASSERT(total_refs);
			uint p = i++ *1000 / total_refs;
			if (p != reported_progress){
				progress(p);
				reported_progress = p;
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
					cs.csumer_for(ref.csum);
				}
				pump(sin, ref.from, nullptr, fname, tmp, num_pumped);
				cs.csumer()->reset();
				sout >> cs;
				pump(sin, ref.to, &sout, fname, tmp, num_pumped);
				if (ref.csum != cs.csumer()->checksum())
					warning( cformat(tr_txt("File {0} is broken."), fname), "Control sums do not match." );
			}
			catch(std::exception &e){
				/* TRANSLATORS: This is about path from and to  */
				warning(cformat(tr_txt("Problem with {0}"), ref.fname), message(e));
			}
		}
	}
	catch(std::exception &e){
		string msg;
		if (name.empty())
			/* TRANSLATORS: This is about path  */
			warning(cformat(tr_txt("Error while testing {0}"), archive_path), message(e));
		else
			/* TRANSLATORS: argument is name */
			warning(cformat(tr_txt("Error while testing {0}"), name), message(e));
	}
}


}
