#include <thread>
#include "coformat.h"

using namespace std;
using namespace coformat;

int main()
{
	try{
		auto colors = to_array<tuple<string, string>>({
			{"black",         "bk"},
			{"red",           "r"},
			{"green",         "g"},
			{"yellow",        "y"},
			{"blue",          "b"},
			{"magenta",       "m"},
			{"cyan",          "c"},
			{"white",         "w"},
			{"bright black",  "bbk"},
			{"bright red",    "br"},
			{"bright green",  "bg"},
			{"bright yellow", "by"},
			{"bright blue",   "bb"},
			{"bright magenta","bm"},
			{"bright cyan",   "bc"},
			{"bright white",  "bw"},
			});
		for (const auto &c : colors){
			string name, code;
			tie(name, code) = c;
			cprintln("{f"+code+"}Hello! {fd}{b"+code+"}      {bd} {}", name);
		}
		auto styles = to_array<tuple<string, string>>({
			{"italic",     "i"},
			{"bold",       "b"},
			{"underline",  "u"},
			{"strike",     "s"},
			{"pulsating",  "p"},
			{"reversed",   "r"},
			});
		for (const auto &s : styles){
			string name, code;
			tie(name, code) = s;
			cprintln("{"+code+"}{}{n"+code+"}", name);
		}
		cprintln("{i}{u}{b}{p}{r}all at once{d}");
		string bar;
		for (int i = 0; i < 100; ++i) {
			if (i % 2)
				bar += "█";
			cprintln("{fy}{:░<50}{fd} {}%", bar, i);
			clear_previous_line();

			this_thread::sleep_for(100ms);

			if (i == 50)
				cprintln("{fr}OOOPs! A warning has been reported.{fd}");
			if (i == 60)
				cprintln("But worry not, progress continues!");
		}
		return 0;
	}
	catch(exception &e){
		println("Error {}: {}", typeid(e).name(), e.what());
	}
	return 1;
}
