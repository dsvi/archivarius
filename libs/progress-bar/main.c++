#include <thread>
#include "progress_bar.h"

using namespace std;
using namespace coformat;

int main()
{
	Progress_bar bar;
	for (int i = bar.min; i < bar.max; ++i) {
		bar.update(i);
		this_thread::sleep_for(100ms);
		if (i == 50)
			cprintln("{fr}OOOPs! A warning has been reported.{fd}");
		if (i == 60)
			cprintln("But worry not, progress continues!");
	}
	return 0;
}
