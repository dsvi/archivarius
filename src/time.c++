#include "time.h"

using namespace std;


namespace archi{

Time Time::from_file_time(std::filesystem::file_time_type file_time)
{
	using namespace chrono;
	// The epoch of the filesystem clock is implementation defined:
	// libc++ counts from 1970, libstdc++ from 2174, MSVC from 1601,
	// so the value has to be converted to the system clock to get POSIX time.
	auto system_time = file_clock::to_sys(file_time);
	return Time(time_point_cast<Accuracy>(system_time).time_since_epoch().count());
}

std::filesystem::file_time_type Time::to_file_time() const
{
	using namespace chrono;
	time_point<system_clock, Accuracy> sys_time{ Accuracy(nanoseconds_) };
	return file_clock::from_sys(sys_time);
}

std::string Time::to_human_readable() const
{
	using namespace chrono;
	auto sys_time = system_clock::time_point{ duration_cast<system_clock::duration>(Accuracy(nanoseconds_)) };
	return format("{:%Y %B %d %H:%M:%S}", time_point_cast<seconds>(sys_time));
}

}
