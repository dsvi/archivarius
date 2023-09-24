#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <time.h>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include <botan_all.h>

#define FMT_CONSTEVAL
#include "cfmt.h"
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/std.h>

#define XXH_CPU_LITTLE_ENDIAN 1
#include "xxhash.h"

#include <google/protobuf/message_lite.h>
#include <google/protobuf/arena.h>

#ifndef NDEBUG
#define DEBUG
#endif

#ifdef DEBUG
#define ASSERTS_ENABLED
#define ASSERT(x) assert(x)
#define COMPRESS_STAT
#else
#define ASSERT(x)
#endif

namespace std{
    template<> struct hash<std::filesystem::path>    {
	      typedef std::filesystem::path argument_type;
	      typedef std::size_t result_type;
	      result_type operator()(argument_type const& a) const noexcept				{
					return std::filesystem::hash_value(a);
				}
    };
}

namespace archi{

/// for scoped enums
template<typename E>
constexpr auto to_int(E e) -> typename std::underlying_type<E>::type
{
	 return static_cast<typename std::underlying_type<E>::type>(e);
}

/// Signed 8-bit integer
typedef int8_t                   s8;
/// Signed 16-bit integer
typedef int16_t                  s16;
/// Signed 32-bit integer
typedef int32_t                  s32;
/// Signed 64-bit integer
typedef int64_t                  s64;

/// Unsigned 8-bit integer
typedef uint8_t                  u8;
/// Unsigned 16-bit integer
typedef uint16_t                 u16;
/// Unsigned 32-bit integer
typedef uint32_t                 u32;
/// Unsigned 64-bit integer
typedef uint64_t                 u64;

typedef unsigned int             uint;

// posix time in nanoseconds
typedef u64 Time;
typedef std::chrono::nanoseconds Time_accuracy;
inline
auto to_sys_clock(Time t){
	 using namespace std::chrono;
	 return system_clock::time_point{duration_cast<system_clock::duration>(Time_accuracy(t))};
}

}
