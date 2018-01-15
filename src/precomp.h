#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <regex>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <experimental/filesystem>


namespace std{
using namespace std::experimental;
}

#include <time.h>

#define BOOST_NO_AUTO_PTR
//#include <boost/algorithm/string.hpp>
//#include <boost/container/flat_map.hpp>
//#include <boost/container/flat_set.hpp>
//#include <boost/container/small_vector.hpp>
//#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#define XXH_CPU_LITTLE_ENDIAN 1
#include <xxhash.hpp>

#include <google/protobuf/message_lite.h>

#ifdef DEBUG
#define ASSERTS_ENABLED
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

/// for scoped enums
template<typename E>
constexpr auto to_int(E e) -> typename std::underlying_type<E>::type
{
	 return static_cast<typename std::underlying_type<E>::type>(e);
}

namespace std{
    template<> struct hash<std::filesystem::path>    {
	      typedef std::filesystem::path argument_type;
	      typedef std::size_t result_type;
	      result_type operator()(argument_type const& a) const noexcept				{
					return std::filesystem::hash_value(a);
				}
    };
}

template<class C, class F>
void erase_if(C &c, F &&f ){
	c.erase(std::remove_if(c.begin(), c.end(), f), c.end());
}

/// Signed 8-bit integer
typedef int8_t                   si8;
/// Signed 16-bit integer
typedef int16_t                  si16;
/// Signed 32-bit integer
typedef int32_t                  si32;
/// Signed 64-bit integer
typedef int64_t                  si64;

/// Unsigned 8-bit integer
typedef uint8_t                  ui8;
/// Unsigned 16-bit integer
typedef uint16_t                 ui16;
/// Unsigned 32-bit integer
typedef uint32_t                 ui32;
/// Unsigned 64-bit integer
typedef uint64_t                 ui64;

typedef unsigned int             uint;
