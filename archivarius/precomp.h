#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <regex>
#include <string_view>
#include <time.h>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include <botan_all.h>

#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/chrono.h>
using namespace fmt::literals;

template <>
struct fmt::formatter<std::filesystem::path> {
	template <typename ParseContext>
	constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const std::filesystem::path &p, FormatContext &ctx) {
		return format_to(ctx.begin(), "{}", p.string());
	}
};


#include <range/v3/all.hpp>

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
using Time = u64;
static const u64 Time_ticks_in_second = 1'000'000'000;

}
