#include "progress_bar.h"
#include <cassert>

using namespace std;
using namespace coformat;

Progress_bar::Progress_bar()
{
}

void Progress_bar::update(uint progress)
{
	if (!is_colorized())
		return;
	assert(min <= progress and progress < max);
	auto new_percent = (progress - min)*100/(max - min);
	if (current_percent == new_percent)
		return;
	current_percent = new_percent;
	percent_str = to_string(current_percent);
	auto bar_len = (progress - min)*width/(max - min);
	bar.clear();
	while (bar_len--)
		bar += bar_filled;
	show();
}

void Progress_bar::show()
{
	if (!is_colorized())
		return;
	if (format_str.empty())
		format_str = format("{{fy}}{{:{}<{}}}{{fd}} {{}}%", bar_empty, width);
	cprintln(format_str, bar, percent_str);
	clear_previous_line();
}
