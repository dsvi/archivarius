#pragma once
#include <coformat.h>


/// draws progress bar at the bottom of the console.
/// does nothing if redirected to a file.
/// disappears when something else is printed to console.
/// appears again on update() or show()
///
/// ████████████████████████████████░░░░░░░░░░░░░░░░░░ 63%
///
/// all parameters can only be set before first show(), except #min and #max
class Progress_bar
{
public:
	Progress_bar();
	std::string bar_filled = "█";
	std::string bar_empty  = "░";
	uint min = 0;
	uint max = 100;
	/// width of the bar in characters.
	uint width = 60;
	/// updates progress and show() it, if percentage has changed
	/// @param progress #min <= progress < #max
	void update(uint progress);
	/// prints indicator to the console
	void show();
private:
	uint current_percent = 0;
	std::string format_str;
	std::string bar;
	std::string percent_str = "0";
};

