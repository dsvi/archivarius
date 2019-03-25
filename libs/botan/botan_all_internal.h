/*
* Botan 2.9.0 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AMALGAMATION_INTERNAL_H_
#define BOTAN_AMALGAMATION_INTERNAL_H_

#include <chrono>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <vector>


namespace Botan {

/**
* If top bit of arg is set, return ~0. Otherwise return 0.
*/
template<typename T>
inline T expand_top_bit(T a)
   {
   return static_cast<T>(0) - (a >> (sizeof(T)*8-1));
   }

/**
* If arg is zero, return ~0. Otherwise return 0
*/
template<typename T>
inline T ct_is_zero(T x)
   {
   return expand_top_bit<T>(~x & (x - 1));
   }

/**
* Power of 2 test. T should be an unsigned integer type
* @param arg an integer value
* @return true iff arg is 2^n for some n > 0
*/
template<typename T>
inline constexpr bool is_power_of_2(T arg)
   {
   return (arg != 0) && (arg != 1) && ((arg & static_cast<T>(arg-1)) == 0);
   }

/**
* Return the index of the highest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the highest set bit in n
*/
template<typename T>
inline size_t high_bit(T n)
   {
   size_t hb = 0;

   for(size_t s = 8*sizeof(T) / 2; s > 0; s /= 2)
      {
      const size_t z = s * ((~ct_is_zero(n >> s)) & 1);
      hb += z;
      n >>= z;
      }

   hb += n;

   return hb;
   }

/**
* Return the number of significant bytes in n
* @param n an integer value
* @return number of significant bytes in n
*/
template<typename T>
inline size_t significant_bytes(T n)
   {
   size_t b = 0;

   for(size_t s = 8*sizeof(n) / 2; s >= 8; s /= 2)
      {
      const size_t z = s * (~ct_is_zero(n >> s) & 1);
      b += z/8;
      n >>= z;
      }

   b += (n != 0);

   return b;
   }

/**
* Count the trailing zero bits in n
* @param n an integer value
* @return maximum x st 2^x divides n
*/
template<typename T>
inline size_t ctz(T n)
   {
   /*
   * If n == 0 then this function will compute 8*sizeof(T)-1, so
   * initialize lb to 1 if n == 0 to produce the expected result.
   */
   size_t lb = ct_is_zero(n) & 1;

   for(size_t s = 8*sizeof(T) / 2; s > 0; s /= 2)
      {
      const T mask = (static_cast<T>(1) << s) - 1;
      const size_t z = s * (ct_is_zero(n & mask) & 1);
      lb += z;
      n >>= z;
      }

   return lb;
   }

template<typename T>
size_t ceil_log2(T x)
   {
   if(x >> (sizeof(T)*8-1))
      return sizeof(T)*8;

   size_t result = 0;
   T compare = 1;

   while(compare < x)
      {
      compare <<= 1;
      result++;
      }

   return result;
   }

// Potentially variable time ctz used for OCB
inline size_t var_ctz32(uint32_t n)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)
   if(n == 0)
      return 32;
   return __builtin_ctz(n);
#else
   return ctz<uint32_t>(n);
#endif
   }

}

namespace Botan {

/**
* Perform encoding using the base provided
* @param base object giving access to the encodings specifications
* @param output an array of at least base.encode_max_output bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding chars will be applied if needed
* @return number of bytes written to output
*/
template <class Base>
size_t base_encode(Base&& base,
                   char output[],
                   const uint8_t input[],
                   size_t input_length,
                   size_t& input_consumed,
                   bool final_inputs)
   {
   input_consumed = 0;

   const size_t encoding_bytes_in = base.encoding_bytes_in();
   const size_t encoding_bytes_out = base.encoding_bytes_out();

   size_t input_remaining = input_length;
   size_t output_produced = 0;

   while(input_remaining >= encoding_bytes_in)
      {
      base.encode(output + output_produced, input + input_consumed);

      input_consumed += encoding_bytes_in;
      output_produced += encoding_bytes_out;
      input_remaining -= encoding_bytes_in;
      }

   if(final_inputs && input_remaining)
      {
      std::vector<uint8_t> remainder(encoding_bytes_in, 0);
      for(size_t i = 0; i != input_remaining; ++i)
         { remainder[i] = input[input_consumed + i]; }

      base.encode(output + output_produced, remainder.data());

      const size_t bits_consumed = base.bits_consumed();
      const size_t remaining_bits_before_padding = base.remaining_bits_before_padding();

      size_t empty_bits = 8 * (encoding_bytes_in - input_remaining);
      size_t index = output_produced + encoding_bytes_out - 1;
      while(empty_bits >= remaining_bits_before_padding)
         {
         output[index--] = '=';
         empty_bits -= bits_consumed;
         }

      input_consumed += input_remaining;
      output_produced += encoding_bytes_out;
      }

   return output_produced;
   }


template <typename Base>
std::string base_encode_to_string(Base&& base, const uint8_t input[], size_t input_length)
   {
   const size_t output_length = base.encode_max_output(input_length);
   std::string output(output_length, 0);

   size_t consumed = 0;
   size_t produced = 0;

   if(output_length > 0)
      {
      produced = base_encode(base, &output.front(),
                                   input, input_length,
                                   consumed, true);
      }

   BOTAN_ASSERT_EQUAL(consumed, input_length, "Consumed the entire input");
   BOTAN_ASSERT_EQUAL(produced, output.size(), "Produced expected size");

   return output;
   }

/**
* Perform decoding using the base provided
* @param base object giving access to the encodings specifications
* @param output an array of at least Base::decode_max_output bytes
* @param input some base input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding is allowed
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
template <typename Base>
size_t base_decode(Base&& base,
                   uint8_t output[],
                   const char input[],
                   size_t input_length,
                   size_t& input_consumed,
                   bool final_inputs,
                   bool ignore_ws = true)
   {
   const size_t decoding_bytes_in = base.decoding_bytes_in();
   const size_t decoding_bytes_out = base.decoding_bytes_out();

   uint8_t* out_ptr = output;
   std::vector<uint8_t> decode_buf(decoding_bytes_in, 0);
   size_t decode_buf_pos = 0;
   size_t final_truncate = 0;

   clear_mem(output, base.decode_max_output(input_length));

   for(size_t i = 0; i != input_length; ++i)
      {
      const uint8_t bin = base.lookup_binary_value(input[i]);

      if(base.check_bad_char(bin, input[i], ignore_ws)) // May throw Invalid_Argument
         {
         decode_buf[decode_buf_pos] = bin;
         ++decode_buf_pos;
         }

      /*
      * If we're at the end of the input, pad with 0s and truncate
      */
      if(final_inputs && (i == input_length - 1))
         {
         if(decode_buf_pos)
            {
            for(size_t j = decode_buf_pos; j < decoding_bytes_in; ++j)
               { decode_buf[j] = 0; }

            final_truncate = decoding_bytes_in - decode_buf_pos;
            decode_buf_pos = decoding_bytes_in;
            }
         }

      if(decode_buf_pos == decoding_bytes_in)
         {
         base.decode(out_ptr, decode_buf.data());

         out_ptr += decoding_bytes_out;
         decode_buf_pos = 0;
         input_consumed = i+1;
         }
      }

   while(input_consumed < input_length &&
         base.lookup_binary_value(input[input_consumed]) == 0x80)
      {
      ++input_consumed;
      }

   size_t written = (out_ptr - output) - base.bytes_to_remove(final_truncate);

   return written;
   }

template<typename Base>
size_t base_decode_full(Base&& base, uint8_t output[], const char input[], size_t input_length, bool ignore_ws)
   {
   size_t consumed = 0;
   const size_t written = base_decode(base, output, input, input_length, consumed, true, ignore_ws);

   if(consumed != input_length)
      {
      throw Invalid_Argument(base.name() + " decoding failed, input did not have full bytes");
      }

   return written;
   }

template<typename Vector, typename Base>
Vector base_decode_to_vec(Base&& base,
                          const char input[],
                          size_t input_length,
                          bool ignore_ws)
   {
   const size_t output_length = base.decode_max_output(input_length);
   Vector bin(output_length);

   const size_t written =
      base_decode_full(base, bin.data(), input, input_length, ignore_ws);

   bin.resize(written);
   return bin;
   }

}

#if defined(BOTAN_HAS_VALGRIND)
  #include <valgrind/memcheck.h>
#endif

namespace Botan {

namespace CT {

/**
* Use valgrind to mark the contents of memory as being undefined.
* Valgrind will accept operations which manipulate undefined values,
* but will warn if an undefined value is used to decided a conditional
* jump or a load/store address. So if we poison all of our inputs we
* can confirm that the operations in question are truly const time
* when compiled by whatever compiler is in use.
*
* Even better, the VALGRIND_MAKE_MEM_* macros work even when the
* program is not run under valgrind (though with a few cycles of
* overhead, which is unfortunate in final binaries as these
* annotations tend to be used in fairly important loops).
*
* This approach was first used in ctgrind (https://github.com/agl/ctgrind)
* but calling the valgrind mecheck API directly works just as well and
* doesn't require a custom patched valgrind.
*/
template<typename T>
inline void poison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_UNDEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(T& p)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(&p, sizeof(T));
#else
   BOTAN_UNUSED(p);
#endif
   }

/**
* A Mask type used for constant-time operations. A Mask<T> always has value
* either 0 (all bits cleared) or ~0 (all bits set). All operations in a Mask<T>
* are intended to compile to code which does not contain conditional jumps.
* This must be verified with tooling (eg binary disassembly or using valgrind)
* since you never know what a compiler might do.
*/
template<typename T>
class Mask
   {
   public:
      static_assert(std::is_unsigned<T>::value, "CT::Mask only defined for unsigned integer types");

      Mask(const Mask<T>& other) = default;
      Mask<T>& operator=(const Mask<T>& other) = default;

      /**
      * Derive a Mask from a Mask of a larger type
      */
      template<typename U>
      Mask(Mask<U> o) : m_mask(static_cast<T>(o.value()))
         {
         static_assert(sizeof(U) > sizeof(T), "sizes ok");
         }

      /**
      * Return a Mask<T> with all bits set
      */
      static Mask<T> set()
         {
         return Mask<T>(static_cast<T>(~0));
         }

      /**
      * Return a Mask<T> with all bits cleared
      */
      static Mask<T> cleared()
         {
         return Mask<T>(0);
         }

      /**
      * Return a Mask<T> which is set if v is != 0
      */
      static Mask<T> expand(T v)
         {
         return ~Mask<T>::is_zero(v);
         }

      /**
      * Return a Mask<T> which is set if m is set
      */
      template<typename U>
      static Mask<T> expand(Mask<U> m)
         {
         static_assert(sizeof(U) < sizeof(T), "sizes ok");
         return ~Mask<T>::is_zero(m.value());
         }

      /**
      * Return a Mask<T> which is set if v is == 0 or cleared otherwise
      */
      static Mask<T> is_zero(T x)
         {
         return Mask<T>(ct_is_zero<T>(x));
         }

      /**
      * Return a Mask<T> which is set if x == y
      */
      static Mask<T> is_equal(T x, T y)
         {
         return Mask<T>::is_zero(static_cast<T>(x ^ y));
         }

      /**
      * Return a Mask<T> which is set if x < y
      */
      static Mask<T> is_lt(T x, T y)
         {
         return Mask<T>(expand_top_bit<T>(x^((x^y) | ((x-y)^x))));
         }

      /**
      * Return a Mask<T> which is set if x > y
      */
      static Mask<T> is_gt(T x, T y)
         {
         return Mask<T>::is_lt(y, x);
         }

      /**
      * Return a Mask<T> which is set if x <= y
      */
      static Mask<T> is_lte(T x, T y)
         {
         return ~Mask<T>::is_gt(x, y);
         }

      /**
      * Return a Mask<T> which is set if x >= y
      */
      static Mask<T> is_gte(T x, T y)
         {
         return ~Mask<T>::is_lt(x, y);
         }

      /**
      * AND-combine two masks
      */
      Mask<T>& operator&=(Mask<T> o)
         {
         m_mask &= o.value();
         return (*this);
         }

      /**
      * XOR-combine two masks
      */
      Mask<T>& operator^=(Mask<T> o)
         {
         m_mask ^= o.value();
         return (*this);
         }

      /**
      * OR-combine two masks
      */
      Mask<T>& operator|=(Mask<T> o)
         {
         m_mask |= o.value();
         return (*this);
         }

      /**
      * AND-combine two masks
      */
      friend Mask<T> operator&(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() & y.value());
         }

      /**
      * XOR-combine two masks
      */
      friend Mask<T> operator^(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() ^ y.value());
         }

      /**
      * OR-combine two masks
      */
      friend Mask<T> operator|(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() | y.value());
         }

      /**
      * Negate this mask
      */
      Mask<T> operator~() const
         {
         return Mask<T>(~value());
         }

      /**
      * Return x if the mask is set, or otherwise zero
      */
      T if_set_return(T x) const
         {
         return m_mask & x;
         }

      /**
      * Return x if the mask is cleared, or otherwise zero
      */
      T if_not_set_return(T x) const
         {
         return ~m_mask & x;
         }

      /**
      * If this mask is set, return x, otherwise return y
      */
      T select(T x, T y) const
         {
         // (x & value()) | (y & ~value())
         return static_cast<T>(y ^ (value() & (x ^ y)));
         }

      T select_and_unpoison(T x, T y) const
         {
         T r = this->select(x, y);
         CT::unpoison(r);
         return r;
         }

      /**
      * If this mask is set, return x, otherwise return y
      */
      Mask<T> select_mask(Mask<T> x, Mask<T> y) const
         {
         return Mask<T>(select(x.value(), y.value()));
         }

      /**
      * Conditionally set output to x or y, depending on if mask is set or
      * cleared (resp)
      */
      void select_n(T output[], const T x[], const T y[], size_t len) const
         {
         for(size_t i = 0; i != len; ++i)
            output[i] = this->select(x[i], y[i]);
         }

      /**
      * If this mask is set, zero out buf, otherwise do nothing
      */
      void if_set_zero_out(T buf[], size_t elems)
         {
         for(size_t i = 0; i != elems; ++i)
            {
            buf[i] = this->if_not_set_return(buf[i]);
            }
         }

      /**
      * Return the value of the mask, unpoisoned
      */
      T unpoisoned_value() const
         {
         T r = value();
         CT::unpoison(r);
         return r;
         }

      /**
      * Return true iff this mask is set
      */
      bool is_set() const
         {
         return unpoisoned_value() != 0;
         }

      /**
      * Return the underlying value of the mask
      */
      T value() const
         {
         return m_mask;
         }

   private:
      Mask(T m) : m_mask(m) {}

      T m_mask;
   };

template<typename T>
inline Mask<T> conditional_copy_mem(T cnd,
                                    T* to,
                                    const T* from0,
                                    const T* from1,
                                    size_t elems)
   {
   const auto mask = CT::Mask<T>::expand(cnd);
   mask.select_n(to, from0, from1, elems);
   return mask;
   }

template<typename T>
inline void conditional_swap(bool cnd, T& x, T& y)
   {
   const auto swap = CT::Mask<T>::expand(cnd);

   T t0 = swap.select(y, x);
   T t1 = swap.select(x, y);
   x = t0;
   y = t1;
   }

template<typename T>
inline void conditional_swap_ptr(bool cnd, T& x, T& y)
   {
   uintptr_t xp = reinterpret_cast<uintptr_t>(x);
   uintptr_t yp = reinterpret_cast<uintptr_t>(y);

   conditional_swap<uintptr_t>(cnd, xp, yp);

   x = reinterpret_cast<T>(xp);
   y = reinterpret_cast<T>(yp);
   }

/**
* If bad_mask is unset, return in[delim_idx:input_length] copied to
* new buffer. If bad_mask is set, return an all zero vector of
* unspecified length.
*/
secure_vector<uint8_t> copy_output(CT::Mask<uint8_t> bad_input,
                                   const uint8_t input[],
                                   size_t input_length,
                                   size_t delim_idx);

secure_vector<uint8_t> strip_leading_zeros(const uint8_t in[], size_t length);

inline secure_vector<uint8_t> strip_leading_zeros(const secure_vector<uint8_t>& in)
   {
   return strip_leading_zeros(in.data(), in.size());
   }

}

}

namespace Botan {

class donna128 final
   {
   public:
      donna128(uint64_t ll = 0, uint64_t hh = 0) { l = ll; h = hh; }

      donna128(const donna128&) = default;
      donna128& operator=(const donna128&) = default;

      friend donna128 operator>>(const donna128& x, size_t shift)
         {
         donna128 z = x;
         if(shift > 0)
            {
            const uint64_t carry = z.h << (64 - shift);
            z.h = (z.h >> shift);
            z.l = (z.l >> shift) | carry;
            }
         return z;
         }

      friend donna128 operator<<(const donna128& x, size_t shift)
         {
         donna128 z = x;
         if(shift > 0)
            {
            const uint64_t carry = z.l >> (64 - shift);
            z.l = (z.l << shift);
            z.h = (z.h << shift) | carry;
            }
         return z;
         }

      friend uint64_t operator&(const donna128& x, uint64_t mask)
         {
         return x.l & mask;
         }

      uint64_t operator&=(uint64_t mask)
         {
         h = 0;
         l &= mask;
         return l;
         }

      donna128& operator+=(const donna128& x)
         {
         l += x.l;
         h += x.h;

         const uint64_t carry = (l < x.l);
         h += carry;
         return *this;
         }

      donna128& operator+=(uint64_t x)
         {
         l += x;
         const uint64_t carry = (l < x);
         h += carry;
         return *this;
         }

      uint64_t lo() const { return l; }
      uint64_t hi() const { return h; }
   private:
      uint64_t h = 0, l = 0;
   };

inline donna128 operator*(const donna128& x, uint64_t y)
   {
   BOTAN_ARG_CHECK(x.hi() == 0, "High 64 bits of donna128 set to zero during multiply");

   uint64_t lo = 0, hi = 0;
   mul64x64_128(x.lo(), y, &lo, &hi);
   return donna128(lo, hi);
   }

inline donna128 operator*(uint64_t y, const donna128& x)
   {
   return x * y;
   }

inline donna128 operator+(const donna128& x, const donna128& y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator+(const donna128& x, uint64_t y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator|(const donna128& x, const donna128& y)
   {
   return donna128(x.lo() | y.lo(), x.hi() | y.hi());
   }

inline uint64_t carry_shift(const donna128& a, size_t shift)
   {
   return (a >> shift).lo();
   }

inline uint64_t combine_lower(const donna128& a, size_t s1,
                              const donna128& b, size_t s2)
   {
   donna128 z = (a >> s1) | (b << s2);
   return z.lo();
   }

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
inline uint64_t carry_shift(const uint128_t a, size_t shift)
   {
   return static_cast<uint64_t>(a >> shift);
   }

inline uint64_t combine_lower(const uint128_t a, size_t s1,
                              const uint128_t b, size_t s2)
   {
   return static_cast<uint64_t>((a >> s1) | (b << s2));
   }
#endif

}

namespace Botan {

/**
* No_Filesystem_Access Exception
*/
class BOTAN_PUBLIC_API(2,0) No_Filesystem_Access final : public Exception
   {
   public:
      No_Filesystem_Access() : Exception("No filesystem access enabled.")
         {}
   };

BOTAN_TEST_API bool has_filesystem_impl();

BOTAN_TEST_API std::vector<std::string> get_files_recursive(const std::string& dir);

}

namespace Botan {

namespace OS {

/*
* This header is internal (not installed) and these functions are not
* intended to be called by applications. However they are given public
* visibility (using BOTAN_TEST_API macro) for the tests. This also probably
* allows them to be overridden by the application on ELF systems, but
* this hasn't been tested.
*/

/**
* @return process ID assigned by the operating system.
* On Unix and Windows systems, this always returns a result
* On IncludeOS it returns 0 since there is no process ID to speak of
* in a unikernel.
*/
uint32_t BOTAN_TEST_API get_process_id();

/**
* Test if we are currently running with elevated permissions
* eg setuid, setgid, or with POSIX caps set.
*/
bool running_in_privileged_state();

/**
* @return CPU processor clock, if available
*
* On Windows, calls QueryPerformanceCounter.
*
* Under GCC or Clang on supported platforms the hardware cycle counter is queried.
* Currently supported processors are x86, PPC, Alpha, SPARC, IA-64, S/390x, and HP-PA.
* If no CPU cycle counter is available on this system, returns zero.
*/
uint64_t BOTAN_TEST_API get_cpu_cycle_counter();

/*
* @return best resolution timestamp available
*
* The epoch and update rate of this clock is arbitrary and depending
* on the hardware it may not tick at a constant rate.
*
* Uses hardware cycle counter, if available.
* On POSIX platforms clock_gettime is used with a monotonic timer
* As a final fallback std::chrono::high_resolution_clock is used.
*/
uint64_t BOTAN_TEST_API get_high_resolution_clock();

/**
* @return system clock (reflecting wall clock) with best resolution
* available, normalized to nanoseconds resolution.
*/
uint64_t BOTAN_TEST_API get_system_timestamp_ns();

/**
* @return maximum amount of memory (in bytes) Botan could/should
* hyptothetically allocate for the memory poool. Reads environment
* variable "BOTAN_MLOCK_POOL_SIZE", set to "0" to disable pool.
*/
size_t get_memory_locking_limit();

/**
* Return the size of a memory page, if that can be derived on the
* current system. Otherwise returns some default value (eg 4096)
*/
size_t system_page_size();

/**
* Read the value of an environment variable. Return nullptr if
* no such variable is set. If the process seems to be running in
* a privileged state (such as setuid) then always returns nullptr,
* similiar to glibc's secure_getenv.
*/
const char* read_env_variable(const std::string& var_name);

/**
* Request so many bytes of page-aligned RAM locked into memory using
* mlock, VirtualLock, or similar. Returns null on failure. The memory
* returned is zeroed. Free it with free_locked_pages.
* @param length requested allocation in bytes
*/
void* allocate_locked_pages(size_t length);

/**
* Free memory allocated by allocate_locked_pages
* @param ptr a pointer returned by allocate_locked_pages
* @param length length passed to allocate_locked_pages
*/
void free_locked_pages(void* ptr, size_t length);

/**
* Run a probe instruction to test for support for a CPU instruction.
* Runs in system-specific env that catches illegal instructions; this
* function always fails if the OS doesn't provide this.
* Returns value of probe_fn, if it could run.
* If error occurs, returns negative number.
* This allows probe_fn to indicate errors of its own, if it wants.
* For example the instruction might not only be only available on some
* CPUs, but also buggy on some subset of these - the probe function
* can test to make sure the instruction works properly before
* indicating that the instruction is available.
*
* @warning on Unix systems uses signal handling in a way that is not
* thread safe. It should only be called in a single-threaded context
* (ie, at static init time).
*
* If probe_fn throws an exception the result is undefined.
*
* Return codes:
* -1 illegal instruction detected
*/
int BOTAN_TEST_API run_cpu_instruction_probe(std::function<int ()> probe_fn);

/**
* Represents a terminal state
*/
class BOTAN_UNSTABLE_API Echo_Suppression
   {
   public:
      /**
      * Reenable echo on this terminal. Can be safely called
      * multiple times. May throw if an error occurs.
      */
      virtual void reenable_echo() = 0;

      /**
      * Implicitly calls reenable_echo, but swallows/ignored all
      * errors which would leave the terminal in an invalid state.
      */
      virtual ~Echo_Suppression() = default;
   };

/**
* Suppress echo on the terminal
* Returns null if this operation is not supported on the current system.
*/
std::unique_ptr<Echo_Suppression> BOTAN_UNSTABLE_API suppress_echo_on_terminal();

}

}

namespace Botan {

template<typename T>
inline void prefetch_readonly(const T* addr, size_t length)
   {
#if defined(__GNUG__)
   const size_t Ts_per_cache_line = CPUID::cache_line_size() / sizeof(T);

   for(size_t i = 0; i <= length; i += Ts_per_cache_line)
      __builtin_prefetch(addr + i, 0);
#endif
   }

template<typename T>
inline void prefetch_readwrite(const T* addr, size_t length)
   {
#if defined(__GNUG__)
   const size_t Ts_per_cache_line = CPUID::cache_line_size() / sizeof(T);

   for(size_t i = 0; i <= length; i += Ts_per_cache_line)
      __builtin_prefetch(addr + i, 1);
#endif
   }

}

namespace Botan {

/**
* Entropy source using the rdrand instruction first introduced on
* Intel's Ivy Bridge architecture.
*/
class Intel_Rdrand final : public Entropy_Source
   {
   public:
      std::string name() const override { return "rdrand"; }
      size_t poll(RandomNumberGenerator& rng) override;
   };

}

namespace Botan {

/**
* Round up
* @param n a non-negative integer
* @param align_to the alignment boundary
* @return n rounded up to a multiple of align_to
*/
inline size_t round_up(size_t n, size_t align_to)
   {
   BOTAN_ARG_CHECK(align_to != 0, "align_to must not be 0");

   if(n % align_to)
      n += align_to - (n % align_to);
   return n;
   }

/**
* Round down
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded down to a multiple of align_to
*/
template<typename T>
inline constexpr T round_down(T n, T align_to)
   {
   return (align_to == 0) ? n : (n - (n % align_to));
   }

/**
* Clamp
*/
inline size_t clamp(size_t n, size_t lower_bound, size_t upper_bound)
   {
   if(n < lower_bound)
      return lower_bound;
   if(n > upper_bound)
      return upper_bound;
   return n;
   }

}

namespace Botan {

class BOTAN_PUBLIC_API(2,0) Integer_Overflow_Detected final : public Exception
   {
   public:
      Integer_Overflow_Detected(const std::string& file, int line) :
         Exception("Integer overflow detected at " + file + ":" + std::to_string(line))
         {}

      ErrorType error_type() const noexcept override { return ErrorType::InternalError; }
   };

inline size_t checked_add(size_t x, size_t y, const char* file, int line)
   {
   // TODO: use __builtin_x_overflow on GCC and Clang
   size_t z = x + y;
   if(z < x)
      {
      throw Integer_Overflow_Detected(file, line);
      }
   return z;
   }

#define BOTAN_CHECKED_ADD(x,y) checked_add(x,y,__FILE__,__LINE__)

}

#if defined(BOTAN_TARGET_SUPPORTS_SSE2)
  #include <emmintrin.h>
  #define BOTAN_SIMD_USE_SSE2

#elif defined(BOTAN_TARGET_SUPPORTS_ALTIVEC)
  #include <altivec.h>
  #undef vector
  #undef bool
  #define BOTAN_SIMD_USE_ALTIVEC

#elif defined(BOTAN_TARGET_SUPPORTS_NEON)
  #include <arm_neon.h>
  #define BOTAN_SIMD_USE_NEON

#else
#endif

namespace Botan {

/**
* 4x32 bit SIMD register
*
* This class is not a general purpose SIMD type, and only offers
* instructions needed for evaluation of specific crypto primitives.
* For example it does not currently have equality operators of any
* kind.
*
* Implemented for SSE2, VMX (Altivec), and NEON.
*/
class SIMD_4x32 final
   {
   public:

      SIMD_4x32& operator=(const SIMD_4x32& other) = default;
      SIMD_4x32(const SIMD_4x32& other) = default;

      SIMD_4x32& operator=(SIMD_4x32&& other) = default;
      SIMD_4x32(SIMD_4x32&& other) = default;

      /**
      * Zero initialize SIMD register with 4 32-bit elements
      */
      SIMD_4x32() // zero initialized
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_setzero_si128();
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_splat_u32(0);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vdupq_n_u32(0);
#else
         m_scalar[0] = 0;
         m_scalar[1] = 0;
         m_scalar[2] = 0;
         m_scalar[3] = 0;
#endif
         }

      /**
      * Load SIMD register with 4 32-bit elements
      */
      explicit SIMD_4x32(const uint32_t B[4])
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_loadu_si128(reinterpret_cast<const __m128i*>(B));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = (__vector unsigned int){B[0], B[1], B[2], B[3]};
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vld1q_u32(B);
#else
         m_scalar[0] = B[0];
         m_scalar[1] = B[1];
         m_scalar[2] = B[2];
         m_scalar[3] = B[3];
#endif
         }

      /**
      * Load SIMD register with 4 32-bit elements
      */
      SIMD_4x32(uint32_t B0, uint32_t B1, uint32_t B2, uint32_t B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_set_epi32(B3, B2, B1, B0);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = (__vector unsigned int){B0, B1, B2, B3};
#elif defined(BOTAN_SIMD_USE_NEON)
         // Better way to do this?
         const uint32_t B[4] = { B0, B1, B2, B3 };
         m_neon = vld1q_u32(B);
#else
         m_scalar[0] = B0;
         m_scalar[1] = B1;
         m_scalar[2] = B2;
         m_scalar[3] = B3;
#endif
         }

      /**
      * Load SIMD register with one 32-bit element repeated
      */
      static SIMD_4x32 splat(uint32_t B)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_set1_epi32(B));
#elif defined(BOTAN_SIMD_USE_ARM)
         return SIMD_4x32(vdupq_n_u32(B));
#else
         return SIMD_4x32(B, B, B, B);
#endif
         }

      /**
      * Load a SIMD register with little-endian convention
      */
      static SIMD_4x32 load_le(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(in)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         uint32_t R[4];
         Botan::load_le(R, static_cast<const uint8_t*>(in), 4);
         return SIMD_4x32(R);

#elif defined(BOTAN_SIMD_USE_NEON)

         SIMD_4x32 l(vld1q_u32(static_cast<const uint32_t*>(in)));
         return CPUID::is_big_endian() ? l.bswap() : l;
#else
         SIMD_4x32 out;
         Botan::load_le(out.m_scalar, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      /**
      * Load a SIMD register with big-endian convention
      */
      static SIMD_4x32 load_be(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         return load_le(in).bswap();

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         uint32_t R[4];
         Botan::load_be(R, static_cast<const uint8_t*>(in), 4);
         return SIMD_4x32(R);

#elif defined(BOTAN_SIMD_USE_NEON)

         SIMD_4x32 l(vld1q_u32(static_cast<const uint32_t*>(in)));
         return CPUID::is_little_endian() ? l.bswap() : l;

#else
         SIMD_4x32 out;
         Botan::load_be(out.m_scalar, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      /**
      * Load a SIMD register with little-endian convention
      */
      void store_le(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         _mm_storeu_si128(reinterpret_cast<__m128i*>(out), m_sse);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;
         vec.V = m_vmx;
         Botan::store_le(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         if(CPUID::is_big_endian())
            {
            bswap().store_le(out);
            }
         else
            {
            vst1q_u8(out, vreinterpretq_u8_u32(m_neon));
            }
#else
         Botan::store_le(out, m_scalar[0], m_scalar[1], m_scalar[2], m_scalar[3]);
#endif
         }

      /**
      * Load a SIMD register with big-endian convention
      */
      void store_be(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         bswap().store_le(out);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;
         vec.V = m_vmx;
         Botan::store_be(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         if(CPUID::is_little_endian())
            {
            bswap().store_le(out);
            }
         else
            {
            vst1q_u8(out, vreinterpretq_u8_u32(m_neon));
            }

#else
         Botan::store_be(out, m_scalar[0], m_scalar[1], m_scalar[2], m_scalar[3]);
#endif
         }


      /*
      * This is used for SHA-2/SHACAL2
      * Return rotr(ROT1) ^ rotr(ROT2) ^ rotr(ROT3)
      */
      template<size_t ROT1, size_t ROT2, size_t ROT3>
      SIMD_4x32 rho() const
         {
         const SIMD_4x32 rot1 = this->rotr<ROT1>();
         const SIMD_4x32 rot2 = this->rotr<ROT2>();
         const SIMD_4x32 rot3 = this->rotr<ROT3>();
         return (rot1 ^ rot2 ^ rot3);
         }

      /**
      * Left rotation by a compile time constant
      */
      template<size_t ROT>
      SIMD_4x32 rotl() const
         {
         static_assert(ROT > 0 && ROT < 32, "Invalid rotation constant");

#if defined(BOTAN_SIMD_USE_SSE2)

         return SIMD_4x32(_mm_or_si128(_mm_slli_epi32(m_sse, static_cast<int>(ROT)),
                                       _mm_srli_epi32(m_sse, static_cast<int>(32-ROT))));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         const unsigned int r = static_cast<unsigned int>(ROT);
         return SIMD_4x32(vec_rl(m_vmx, (__vector unsigned int){r, r, r, r}));

#elif defined(BOTAN_SIMD_USE_NEON)

         #if defined(BOTAN_TARGET_ARCH_IS_ARM32)

         return SIMD_4x32(vorrq_u32(vshlq_n_u32(m_neon, static_cast<int>(ROT)),
                                    vshrq_n_u32(m_neon, static_cast<int>(32-ROT))));

         #else

         if(ROT == 8)
            {
            const uint8_t maskb[16] = { 3,0,1,2, 7,4,5,6, 11,8,9,10, 15,12,13,14 };
            const uint8x16_t mask = vld1q_u8(maskb);
            return SIMD_4x32(vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(m_neon), mask)));
            }
         else if(ROT == 16)
            {
            return SIMD_4x32(vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(m_neon))));
            }
         else
            {
            return SIMD_4x32(vorrq_u32(vshlq_n_u32(m_neon, static_cast<int>(ROT)),
                                       vshrq_n_u32(m_neon, static_cast<int>(32-ROT))));
            }

         #endif

#else
         return SIMD_4x32(Botan::rotl<ROT>(m_scalar[0]),
                          Botan::rotl<ROT>(m_scalar[1]),
                          Botan::rotl<ROT>(m_scalar[2]),
                          Botan::rotl<ROT>(m_scalar[3]));
#endif
         }

      /**
      * Right rotation by a compile time constant
      */
      template<size_t ROT>
      SIMD_4x32 rotr() const
         {
         return this->rotl<32-ROT>();
         }

      /**
      * Add elements of a SIMD vector
      */
      SIMD_4x32 operator+(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval += other;
         return retval;
         }

      /**
      * Subtract elements of a SIMD vector
      */
      SIMD_4x32 operator-(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval -= other;
         return retval;
         }

      /**
      * XOR elements of a SIMD vector
      */
      SIMD_4x32 operator^(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval ^= other;
         return retval;
         }

      /**
      * Binary OR elements of a SIMD vector
      */
      SIMD_4x32 operator|(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval |= other;
         return retval;
         }

      /**
      * Binary AND elements of a SIMD vector
      */
      SIMD_4x32 operator&(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval &= other;
         return retval;
         }

      void operator+=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_add_epi32(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_add(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vaddq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] += other.m_scalar[0];
         m_scalar[1] += other.m_scalar[1];
         m_scalar[2] += other.m_scalar[2];
         m_scalar[3] += other.m_scalar[3];
#endif
         }

      void operator-=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_sub_epi32(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_sub(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vsubq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] -= other.m_scalar[0];
         m_scalar[1] -= other.m_scalar[1];
         m_scalar[2] -= other.m_scalar[2];
         m_scalar[3] -= other.m_scalar[3];
#endif
         }

      void operator^=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_xor_si128(m_sse, other.m_sse);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_xor(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = veorq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] ^= other.m_scalar[0];
         m_scalar[1] ^= other.m_scalar[1];
         m_scalar[2] ^= other.m_scalar[2];
         m_scalar[3] ^= other.m_scalar[3];
#endif
         }

      void operator|=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_or_si128(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_or(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vorrq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] |= other.m_scalar[0];
         m_scalar[1] |= other.m_scalar[1];
         m_scalar[2] |= other.m_scalar[2];
         m_scalar[3] |= other.m_scalar[3];
#endif
         }

      void operator&=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_and_si128(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_and(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vandq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] &= other.m_scalar[0];
         m_scalar[1] &= other.m_scalar[1];
         m_scalar[2] &= other.m_scalar[2];
         m_scalar[3] &= other.m_scalar[3];
#endif
         }


      template<int SHIFT> SIMD_4x32 shl() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_slli_epi32(m_sse, SHIFT));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(SHIFT);
         return SIMD_4x32(vec_sl(m_vmx, (__vector unsigned int){s, s, s, s}));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vshlq_n_u32(m_neon, SHIFT));
#else
         return SIMD_4x32(m_scalar[0] << SHIFT,
                          m_scalar[1] << SHIFT,
                          m_scalar[2] << SHIFT,
                          m_scalar[3] << SHIFT);
#endif
         }

      template<int SHIFT> SIMD_4x32 shr() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_srli_epi32(m_sse, SHIFT));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(SHIFT);
         return SIMD_4x32(vec_sr(m_vmx, (__vector unsigned int){s, s, s, s}));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vshrq_n_u32(m_neon, SHIFT));
#else
         return SIMD_4x32(m_scalar[0] >> SHIFT, m_scalar[1] >> SHIFT,
                          m_scalar[2] >> SHIFT, m_scalar[3] >> SHIFT);

#endif
         }

      SIMD_4x32 operator~() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_xor_si128(m_sse, _mm_set1_epi32(0xFFFFFFFF)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_nor(m_vmx, m_vmx));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vmvnq_u32(m_neon));
#else
         return SIMD_4x32(~m_scalar[0], ~m_scalar[1], ~m_scalar[2], ~m_scalar[3]);
#endif
         }

      // (~reg) & other
      SIMD_4x32 andc(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_andnot_si128(m_sse, other.m_sse));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         /*
         AltiVec does arg1 & ~arg2 rather than SSE's ~arg1 & arg2
         so swap the arguments
         */
         return SIMD_4x32(vec_andc(other.m_vmx, m_vmx));
#elif defined(BOTAN_SIMD_USE_NEON)
         // NEON is also a & ~b
         return SIMD_4x32(vbicq_u32(other.m_neon, m_neon));
#else
         return SIMD_4x32((~m_scalar[0]) & other.m_scalar[0],
                          (~m_scalar[1]) & other.m_scalar[1],
                          (~m_scalar[2]) & other.m_scalar[2],
                          (~m_scalar[3]) & other.m_scalar[3]);
#endif
         }

      /**
      * Return copy *this with each word byte swapped
      */
      SIMD_4x32 bswap() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         __m128i T = m_sse;
         T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         return SIMD_4x32(_mm_or_si128(_mm_srli_epi16(T, 8), _mm_slli_epi16(T, 8)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;

         vec.V = m_vmx;
         bswap_4(vec.R);
         return SIMD_4x32(vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         return SIMD_4x32(vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(m_neon))));

#else
         // scalar
         return SIMD_4x32(reverse_bytes(m_scalar[0]),
                          reverse_bytes(m_scalar[1]),
                          reverse_bytes(m_scalar[2]),
                          reverse_bytes(m_scalar[3]));
#endif
         }

      /**
      * 4x4 Transposition on SIMD registers
      */
      static void transpose(SIMD_4x32& B0, SIMD_4x32& B1,
                            SIMD_4x32& B2, SIMD_4x32& B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         const __m128i T0 = _mm_unpacklo_epi32(B0.m_sse, B1.m_sse);
         const __m128i T1 = _mm_unpacklo_epi32(B2.m_sse, B3.m_sse);
         const __m128i T2 = _mm_unpackhi_epi32(B0.m_sse, B1.m_sse);
         const __m128i T3 = _mm_unpackhi_epi32(B2.m_sse, B3.m_sse);

         B0.m_sse = _mm_unpacklo_epi64(T0, T1);
         B1.m_sse = _mm_unpackhi_epi64(T0, T1);
         B2.m_sse = _mm_unpacklo_epi64(T2, T3);
         B3.m_sse = _mm_unpackhi_epi64(T2, T3);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned int T0 = vec_mergeh(B0.m_vmx, B2.m_vmx);
         const __vector unsigned int T1 = vec_mergeh(B1.m_vmx, B3.m_vmx);
         const __vector unsigned int T2 = vec_mergel(B0.m_vmx, B2.m_vmx);
         const __vector unsigned int T3 = vec_mergel(B1.m_vmx, B3.m_vmx);

         B0.m_vmx = vec_mergeh(T0, T1);
         B1.m_vmx = vec_mergel(T0, T1);
         B2.m_vmx = vec_mergeh(T2, T3);
         B3.m_vmx = vec_mergel(T2, T3);
#elif defined(BOTAN_SIMD_USE_NEON)

#if defined(BOTAN_TARGET_ARCH_IS_ARM32)

         const uint32x4x2_t T0 = vzipq_u32(B0.m_neon, B2.m_neon);
         const uint32x4x2_t T1 = vzipq_u32(B1.m_neon, B3.m_neon);
         const uint32x4x2_t O0 = vzipq_u32(T0.val[0], T1.val[0]);
         const uint32x4x2_t O1 = vzipq_u32(T0.val[1], T1.val[1]);

         B0.m_neon = O0.val[0];
         B1.m_neon = O0.val[1];
         B2.m_neon = O1.val[0];
         B3.m_neon = O1.val[1];

#elif defined(BOTAN_TARGET_ARCH_IS_ARM64)
         const uint32x4_t T0 = vzip1q_u32(B0.m_neon, B2.m_neon);
         const uint32x4_t T2 = vzip2q_u32(B0.m_neon, B2.m_neon);

         const uint32x4_t T1 = vzip1q_u32(B1.m_neon, B3.m_neon);
         const uint32x4_t T3 = vzip2q_u32(B1.m_neon, B3.m_neon);

         B0.m_neon = vzip1q_u32(T0, T1);
         B1.m_neon = vzip2q_u32(T0, T1);

         B2.m_neon = vzip1q_u32(T2, T3);
         B3.m_neon = vzip2q_u32(T2, T3);
#endif

#else
         // scalar
         SIMD_4x32 T0(B0.m_scalar[0], B1.m_scalar[0], B2.m_scalar[0], B3.m_scalar[0]);
         SIMD_4x32 T1(B0.m_scalar[1], B1.m_scalar[1], B2.m_scalar[1], B3.m_scalar[1]);
         SIMD_4x32 T2(B0.m_scalar[2], B1.m_scalar[2], B2.m_scalar[2], B3.m_scalar[2]);
         SIMD_4x32 T3(B0.m_scalar[3], B1.m_scalar[3], B2.m_scalar[3], B3.m_scalar[3]);

         B0 = T0;
         B1 = T1;
         B2 = T2;
         B3 = T3;
#endif
         }

   private:

#if defined(BOTAN_SIMD_USE_SSE2)
      explicit SIMD_4x32(__m128i in) : m_sse(in) {}
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      explicit SIMD_4x32(__vector unsigned int in) : m_vmx(in) {}
#elif defined(BOTAN_SIMD_USE_NEON)
      explicit SIMD_4x32(uint32x4_t in) : m_neon(in) {}
#endif

#if defined(BOTAN_SIMD_USE_SSE2)
      __m128i m_sse;
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      __vector unsigned int m_vmx;
#elif defined(BOTAN_SIMD_USE_NEON)
      uint32x4_t m_neon;
#else
      uint32_t m_scalar[4];
#endif
   };

}
#include <immintrin.h>

namespace Botan {

class SIMD_8x32 final
   {
   public:

      SIMD_8x32& operator=(const SIMD_8x32& other) = default;
      SIMD_8x32(const SIMD_8x32& other) = default;

      SIMD_8x32& operator=(SIMD_8x32&& other) = default;
      SIMD_8x32(SIMD_8x32&& other) = default;

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32()
         {
         m_avx2 = _mm256_setzero_si256();
         }

      BOTAN_FUNC_ISA("avx2")
      explicit SIMD_8x32(const uint32_t B[8])
         {
         m_avx2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(B));
         }

      BOTAN_FUNC_ISA("avx2")
      explicit SIMD_8x32(uint32_t B0, uint32_t B1, uint32_t B2, uint32_t B3,
                         uint32_t B4, uint32_t B5, uint32_t B6, uint32_t B7)
         {
         m_avx2 = _mm256_set_epi32(B7, B6, B5, B4, B3, B2, B1, B0);
         }

      BOTAN_FUNC_ISA("avx2")
      static SIMD_8x32 splat(uint32_t B)
         {
         return SIMD_8x32(_mm256_set1_epi32(B));
         }

      BOTAN_FUNC_ISA("avx2")
      static SIMD_8x32 load_le(const uint8_t* in)
         {
         return SIMD_8x32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(in)));
         }

      BOTAN_FUNC_ISA("avx2")
      static SIMD_8x32 load_be(const uint8_t* in)
         {
         return load_le(in).bswap();
         }

      BOTAN_FUNC_ISA("avx2")
      void store_le(uint8_t out[]) const
         {
         _mm256_storeu_si256(reinterpret_cast<__m256i*>(out), m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void store_be(uint8_t out[]) const
         {
         bswap().store_le(out);
         }

      template<size_t ROT>
      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 rotl() const
         {
         static_assert(ROT > 0 && ROT < 32, "Invalid rotation constant");

         if(ROT == 8)
            {
            const __m256i shuf_rotl_8 = _mm256_set_epi8(14, 13, 12, 15, 10, 9, 8, 11, 6, 5, 4, 7, 2, 1, 0, 3,
                                                        14, 13, 12, 15, 10, 9, 8, 11, 6, 5, 4, 7, 2, 1, 0, 3);

            return SIMD_8x32(_mm256_shuffle_epi8(m_avx2, shuf_rotl_8));
            }
         else if(ROT == 16)
            {
            const __m256i shuf_rotl_16 = _mm256_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2,
                                                         13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

            return SIMD_8x32(_mm256_shuffle_epi8(m_avx2, shuf_rotl_16));
            }
         else
            {
            return SIMD_8x32(_mm256_or_si256(_mm256_slli_epi32(m_avx2, static_cast<int>(ROT)),
                                             _mm256_srli_epi32(m_avx2, static_cast<int>(32-ROT))));
            }
         }

      template<size_t ROT>
      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 rotr() const
         {
         return this->rotl<32-ROT>();
         }

      SIMD_8x32 operator+(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval += other;
         return retval;
         }

      SIMD_8x32 operator-(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval -= other;
         return retval;
         }

      SIMD_8x32 operator^(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval ^= other;
         return retval;
         }

      SIMD_8x32 operator|(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval |= other;
         return retval;
         }

      SIMD_8x32 operator&(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval &= other;
         return retval;
         }

      BOTAN_FUNC_ISA("avx2")
      void operator+=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_add_epi32(m_avx2, other.m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void operator-=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_sub_epi32(m_avx2, other.m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void operator^=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_xor_si256(m_avx2, other.m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void operator|=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_or_si256(m_avx2, other.m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void operator&=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_and_si256(m_avx2, other.m_avx2);
         }

      template<int SHIFT> BOTAN_FUNC_ISA("avx2") SIMD_8x32 shl() const
         {
         return SIMD_8x32(_mm256_slli_epi32(m_avx2, SHIFT));
         }

      template<int SHIFT> BOTAN_FUNC_ISA("avx2")SIMD_8x32 shr() const
         {
         return SIMD_8x32(_mm256_srli_epi32(m_avx2, SHIFT));
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 operator~() const
         {
         return SIMD_8x32(_mm256_xor_si256(m_avx2, _mm256_set1_epi32(0xFFFFFFFF)));
         }

      // (~reg) & other
      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 andc(const SIMD_8x32& other) const
         {
         return SIMD_8x32(_mm256_andnot_si256(m_avx2, other.m_avx2));
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 bswap() const
         {
         const uint8_t BSWAP_MASK[32] = { 3, 2, 1, 0,
                                          7, 6, 5, 4,
                                          11, 10, 9, 8,
                                          15, 14, 13, 12,
                                          19, 18, 17, 16,
                                          23, 22, 21, 20,
                                          27, 26, 25, 24,
                                          31, 30, 29, 28 };

         const __m256i bswap = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(BSWAP_MASK));

         const __m256i output = _mm256_shuffle_epi8(m_avx2, bswap);

         return SIMD_8x32(output);
         }

      BOTAN_FUNC_ISA("avx2")
      static void transpose(SIMD_8x32& B0, SIMD_8x32& B1,
                            SIMD_8x32& B2, SIMD_8x32& B3)
         {
         const __m256i T0 = _mm256_unpacklo_epi32(B0.m_avx2, B1.m_avx2);
         const __m256i T1 = _mm256_unpacklo_epi32(B2.m_avx2, B3.m_avx2);
         const __m256i T2 = _mm256_unpackhi_epi32(B0.m_avx2, B1.m_avx2);
         const __m256i T3 = _mm256_unpackhi_epi32(B2.m_avx2, B3.m_avx2);

         B0.m_avx2 = _mm256_unpacklo_epi64(T0, T1);
         B1.m_avx2 = _mm256_unpackhi_epi64(T0, T1);
         B2.m_avx2 = _mm256_unpacklo_epi64(T2, T3);
         B3.m_avx2 = _mm256_unpackhi_epi64(T2, T3);
         }

      BOTAN_FUNC_ISA("avx2")
      static void reset_registers()
         {
         _mm256_zeroupper();
         }

      BOTAN_FUNC_ISA("avx2")
      static void zero_registers()
         {
         _mm256_zeroall();
         }

      __m256i handle() const { return m_avx2; }

   private:

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32(__m256i x) : m_avx2(x) {}

      __m256i m_avx2;
   };

}

namespace Botan {

inline std::vector<uint8_t> to_byte_vector(const std::string& s)
   {
   return std::vector<uint8_t>(s.cbegin(), s.cend());
   }

inline std::string to_string(const secure_vector<uint8_t> &bytes)
   {
   return std::string(bytes.cbegin(), bytes.cend());
   }

/**
* Return the keys of a map as a std::set
*/
template<typename K, typename V>
std::set<K> map_keys_as_set(const std::map<K, V>& kv)
   {
   std::set<K> s;
   for(auto&& i : kv)
      {
      s.insert(i.first);
      }
   return s;
   }

/*
* Searching through a std::map
* @param mapping the map to search
* @param key is what to look for
* @param null_result is the value to return if key is not in mapping
* @return mapping[key] or null_result
*/
template<typename K, typename V>
inline V search_map(const std::map<K, V>& mapping,
                    const K& key,
                    const V& null_result = V())
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return i->second;
   }

template<typename K, typename V, typename R>
inline R search_map(const std::map<K, V>& mapping, const K& key,
                    const R& null_result, const R& found_result)
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return found_result;
   }

/*
* Insert a key/value pair into a multimap
*/
template<typename K, typename V>
void multimap_insert(std::multimap<K, V>& multimap,
                     const K& key, const V& value)
   {
   multimap.insert(std::make_pair(key, value));
   }

/**
* Existence check for values
*/
template<typename T>
bool value_exists(const std::vector<T>& vec,
                  const T& val)
   {
   for(size_t i = 0; i != vec.size(); ++i)
      if(vec[i] == val)
         return true;
   return false;
   }

template<typename T, typename Pred>
void map_remove_if(Pred pred, T& assoc)
   {
   auto i = assoc.begin();
   while(i != assoc.end())
      {
      if(pred(i->first))
         assoc.erase(i++);
      else
         i++;
      }
   }

}

namespace Botan {

class BOTAN_TEST_API Timer final
   {
   public:
      Timer(const std::string& name,
            const std::string& provider,
            const std::string& doing,
            uint64_t event_mult,
            size_t buf_size,
            double clock_cycle_ratio,
            uint64_t clock_speed)
         : m_name(name + ((provider.empty() || provider == "base") ? "" : " [" + provider + "]"))
         , m_doing(doing)
         , m_buf_size(buf_size)
         , m_event_mult(event_mult)
         , m_clock_cycle_ratio(clock_cycle_ratio)
         , m_clock_speed(clock_speed)
         {}

      Timer(const std::string& name) :
         Timer(name, "", "", 1, 0, 0.0, 0)
         {}

      Timer(const std::string& name, size_t buf_size) :
         Timer(name, "", "", buf_size, buf_size, 0.0, 0)
         {}

      Timer(const Timer& other) = default;

      void start();

      void stop();

      bool under(std::chrono::milliseconds msec)
         {
         return (milliseconds() < msec.count());
         }

      class Timer_Scope final
         {
         public:
            explicit Timer_Scope(Timer& timer)
               : m_timer(timer)
               {
               m_timer.start();
               }
            ~Timer_Scope()
               {
               try
                  {
                  m_timer.stop();
                  }
               catch(...) {}
               }
         private:
            Timer& m_timer;
         };

      template<typename F>
      auto run(F f) -> decltype(f())
         {
         Timer_Scope timer(*this);
         return f();
         }

      template<typename F>
      void run_until_elapsed(std::chrono::milliseconds msec, F f)
         {
         while(this->under(msec))
            {
            run(f);
            }
         }

      uint64_t value() const
         {
         return m_time_used;
         }

      double seconds() const
         {
         return milliseconds() / 1000.0;
         }

      double milliseconds() const
         {
         return value() / 1000000.0;
         }

      double ms_per_event() const
         {
         return milliseconds() / events();
         }

      uint64_t cycles_consumed() const
         {
         if(m_clock_speed != 0)
            {
            return static_cast<uint64_t>((m_clock_speed * value()) / 1000.0);
            }
         return m_cpu_cycles_used;
         }

      uint64_t events() const
         {
         return m_event_count * m_event_mult;
         }

      const std::string& get_name() const
         {
         return m_name;
         }

      const std::string& doing() const
         {
         return m_doing;
         }

      size_t buf_size() const
         {
         return m_buf_size;
         }

      double bytes_per_second() const
         {
         return seconds() > 0.0 ? events() / seconds() : 0.0;
         }

      double events_per_second() const
         {
         return seconds() > 0.0 ? events() / seconds() : 0.0;
         }

      double seconds_per_event() const
         {
         return events() > 0 ? seconds() / events() : 0.0;
         }

      void set_custom_msg(const std::string& s)
         {
         m_custom_msg = s;
         }

      bool operator<(const Timer& other) const;

      std::string to_string() const;

   private:
      std::string result_string_bps() const;
      std::string result_string_ops() const;

      // const data
      std::string m_name, m_doing;
      size_t m_buf_size;
      uint64_t m_event_mult;
      double m_clock_cycle_ratio;
      uint64_t m_clock_speed;

      // set at runtime
      std::string m_custom_msg;
      uint64_t m_time_used = 0, m_timer_start = 0;
      uint64_t m_event_count = 0;

      uint64_t m_max_time = 0, m_min_time = 0;
      uint64_t m_cpu_cycles_start = 0, m_cpu_cycles_used = 0;
   };

}

#endif // BOTAN_AMALGAMATION_INTERNAL_H_
