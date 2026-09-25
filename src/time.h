#pragma once
#include "precomp.h"

namespace archi{


/*
POSIX time: nanoseconds since the epoch.
*/
class Time
{
public:
	/// duration type of the stored time
	using Accuracy = std::chrono::nanoseconds;

	constexpr Time() = default;
	/// from nanoseconds since the POSIX epoch
	constexpr explicit Time(u64 nanoseconds);

	/// from an amount of seconds
	static constexpr Time from_seconds(u64 seconds);
	/// from a filesystem clock time
	static Time from_file_time(std::filesystem::file_time_type file_time);

	/// nanoseconds since the POSIX epoch
	constexpr u64 nanoseconds() const;

	std::filesystem::file_time_type to_file_time() const;
	/// e.g. "2026 September 25 06:22:20" - whole seconds, in UTC
	std::string to_human_readable() const;

	constexpr Time operator-(const Time &t) const;
	constexpr bool operator==(const Time &t) const;
	constexpr bool operator<(const Time &t) const;

private:
	static constexpr u64 nanoseconds_per_second = 1000000000;

	u64 nanoseconds_ = 0;
};

inline
constexpr Time::Time(u64 nanoseconds)
	: nanoseconds_(nanoseconds)
{
}

inline
constexpr Time Time::from_seconds(u64 seconds)
{
	return Time(seconds * nanoseconds_per_second);
}

inline
constexpr u64 Time::nanoseconds() const
{
	return nanoseconds_;
}

inline
constexpr Time Time::operator-(const Time &t) const
{
	return Time(nanoseconds_ - t.nanoseconds_);
}

inline
constexpr bool Time::operator==(const Time &t) const
{
	return nanoseconds_ == t.nanoseconds_;
}

inline
constexpr bool Time::operator<(const Time &t) const
{
	return nanoseconds_ < t.nanoseconds_;
}

}
