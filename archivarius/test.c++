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
		for (size_t i = 0; i < cat.num_states(); i++)
			cat.fs_state(i);
		File_source in;
		Stream_in sin;
		Stream_out sout;
		Filtrator_in filters;
		Checksumer cs;
		decltype(File_content_ref::fname) fname;
		decltype(File_content_ref::from)  num_pumped;
		for (auto ref : cat.content_refs()){
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
