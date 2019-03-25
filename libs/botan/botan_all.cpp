/*
* Botan 2.9.0 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

/*
* (C) 2013,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <sstream>

#if defined(BOTAN_HAS_BLOCK_CIPHER)
#endif

#if defined(BOTAN_HAS_AEAD_CCM)
#endif

#if defined(BOTAN_HAS_AEAD_CHACHA20_POLY1305)
#endif

#if defined(BOTAN_HAS_AEAD_EAX)
#endif

#if defined(BOTAN_HAS_AEAD_GCM)
#endif

#if defined(BOTAN_HAS_AEAD_OCB)
#endif

#if defined(BOTAN_HAS_AEAD_SIV)
#endif

namespace Botan {

std::unique_ptr<AEAD_Mode> AEAD_Mode::create_or_throw(const std::string& algo,
                                                      Cipher_Dir dir,
                                                      const std::string& provider)
   {
   if(auto aead = AEAD_Mode::create(algo, dir, provider))
      return aead;

   throw Lookup_Error("AEAD", algo, provider);
   }

std::unique_ptr<AEAD_Mode> AEAD_Mode::create(const std::string& algo,
                                             Cipher_Dir dir,
                                             const std::string& provider)
   {
#if defined(BOTAN_HAS_AEAD_CHACHA20_POLY1305)
   if(algo == "ChaCha20Poly1305")
      {
      if(dir == ENCRYPTION)
         return std::unique_ptr<AEAD_Mode>(new ChaCha20Poly1305_Encryption);
      else
         return std::unique_ptr<AEAD_Mode>(new ChaCha20Poly1305_Decryption);

      }
#endif

   if(algo.find('/') != std::string::npos)
      {
      const std::vector<std::string> algo_parts = split_on(algo, '/');
      const std::string cipher_name = algo_parts[0];
      const std::vector<std::string> mode_info = parse_algorithm_name(algo_parts[1]);

      if(mode_info.empty())
         return std::unique_ptr<AEAD_Mode>();

      std::ostringstream alg_args;

      alg_args << '(' << cipher_name;
      for(size_t i = 1; i < mode_info.size(); ++i)
         alg_args << ',' << mode_info[i];
      for(size_t i = 2; i < algo_parts.size(); ++i)
         alg_args << ',' << algo_parts[i];
      alg_args << ')';

      const std::string mode_name = mode_info[0] + alg_args.str();
      return AEAD_Mode::create(mode_name, dir);
      }

#if defined(BOTAN_HAS_BLOCK_CIPHER)

   SCAN_Name req(algo);

   if(req.arg_count() == 0)
      {
      return std::unique_ptr<AEAD_Mode>();
      }

   std::unique_ptr<BlockCipher> bc(BlockCipher::create(req.arg(0), provider));

   if(!bc)
      {
      return std::unique_ptr<AEAD_Mode>();
      }

#if defined(BOTAN_HAS_AEAD_CCM)
   if(req.algo_name() == "CCM")
      {
      size_t tag_len = req.arg_as_integer(1, 16);
      size_t L_len = req.arg_as_integer(2, 3);
      if(dir == ENCRYPTION)
         return std::unique_ptr<AEAD_Mode>(new CCM_Encryption(bc.release(), tag_len, L_len));
      else
         return std::unique_ptr<AEAD_Mode>(new CCM_Decryption(bc.release(), tag_len, L_len));
      }
#endif

#if defined(BOTAN_HAS_AEAD_GCM)
   if(req.algo_name() == "GCM")
      {
      size_t tag_len = req.arg_as_integer(1, 16);
      if(dir == ENCRYPTION)
         return std::unique_ptr<AEAD_Mode>(new GCM_Encryption(bc.release(), tag_len));
      else
         return std::unique_ptr<AEAD_Mode>(new GCM_Decryption(bc.release(), tag_len));
      }
#endif

#if defined(BOTAN_HAS_AEAD_OCB)
   if(req.algo_name() == "OCB")
      {
      size_t tag_len = req.arg_as_integer(1, 16);
      if(dir == ENCRYPTION)
         return std::unique_ptr<AEAD_Mode>(new OCB_Encryption(bc.release(), tag_len));
      else
         return std::unique_ptr<AEAD_Mode>(new OCB_Decryption(bc.release(), tag_len));
      }
#endif

#if defined(BOTAN_HAS_AEAD_EAX)
   if(req.algo_name() == "EAX")
      {
      size_t tag_len = req.arg_as_integer(1, bc->block_size());
      if(dir == ENCRYPTION)
         return std::unique_ptr<AEAD_Mode>(new EAX_Encryption(bc.release(), tag_len));
      else
         return std::unique_ptr<AEAD_Mode>(new EAX_Decryption(bc.release(), tag_len));
      }
#endif

#if defined(BOTAN_HAS_AEAD_SIV)
   if(req.algo_name() == "SIV")
      {
      if(dir == ENCRYPTION)
         return std::unique_ptr<AEAD_Mode>(new SIV_Encryption(bc.release()));
      else
         return std::unique_ptr<AEAD_Mode>(new SIV_Decryption(bc.release()));
      }
#endif

#endif

   return std::unique_ptr<AEAD_Mode>();
   }



}
/*
* SCAN Name Abstraction
* (C) 2008-2009,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

std::string make_arg(
   const std::vector<std::pair<size_t, std::string> >& name, size_t start)
   {
   std::string output = name[start].second;
   size_t level = name[start].first;

   size_t paren_depth = 0;

   for(size_t i = start + 1; i != name.size(); ++i)
      {
      if(name[i].first <= name[start].first)
         break;

      if(name[i].first > level)
         {
         output += "(" + name[i].second;
         ++paren_depth;
         }
      else if(name[i].first < level)
         {
         output += ")," + name[i].second;
         --paren_depth;
         }
      else
         {
         if(output[output.size() - 1] != '(')
            output += ",";
         output += name[i].second;
         }

      level = name[i].first;
      }

   for(size_t i = 0; i != paren_depth; ++i)
      output += ")";

   return output;
   }

}

SCAN_Name::SCAN_Name(const char* algo_spec) : SCAN_Name(std::string(algo_spec))
   {
   }

SCAN_Name::SCAN_Name(std::string algo_spec) : m_orig_algo_spec(algo_spec), m_alg_name(), m_args(), m_mode_info()
   {
   std::vector<std::pair<size_t, std::string> > name;
   size_t level = 0;
   std::pair<size_t, std::string> accum = std::make_pair(level, "");

   const std::string decoding_error = "Bad SCAN name '" + algo_spec + "': ";

   for(size_t i = 0; i != algo_spec.size(); ++i)
      {
      char c = algo_spec[i];

      if(c == '/' || c == ',' || c == '(' || c == ')')
         {
         if(c == '(')
            ++level;
         else if(c == ')')
            {
            if(level == 0)
               throw Decoding_Error(decoding_error + "Mismatched parens");
            --level;
            }

         if(c == '/' && level > 0)
            accum.second.push_back(c);
         else
            {
            if(accum.second != "")
               name.push_back(accum);
            accum = std::make_pair(level, "");
            }
         }
      else
         accum.second.push_back(c);
      }

   if(accum.second != "")
      name.push_back(accum);

   if(level != 0)
      throw Decoding_Error(decoding_error + "Missing close paren");

   if(name.size() == 0)
      throw Decoding_Error(decoding_error + "Empty name");

   m_alg_name = name[0].second;

   bool in_modes = false;

   for(size_t i = 1; i != name.size(); ++i)
      {
      if(name[i].first == 0)
         {
         m_mode_info.push_back(make_arg(name, i));
         in_modes = true;
         }
      else if(name[i].first == 1 && !in_modes)
         m_args.push_back(make_arg(name, i));
      }
   }

std::string SCAN_Name::arg(size_t i) const
   {
   if(i >= arg_count())
      throw Invalid_Argument("SCAN_Name::arg " + std::to_string(i) +
                             " out of range for '" + as_string() + "'");
   return m_args[i];
   }

std::string SCAN_Name::arg(size_t i, const std::string& def_value) const
   {
   if(i >= arg_count())
      return def_value;
   return m_args[i];
   }

size_t SCAN_Name::arg_as_integer(size_t i, size_t def_value) const
   {
   if(i >= arg_count())
      return def_value;
   return to_u32bit(m_args[i]);
   }

}
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void SymmetricAlgorithm::throw_key_not_set_error() const
   {
   throw Key_Not_Set(name());
   }

void SymmetricAlgorithm::set_key(const uint8_t key[], size_t length)
   {
   if(!valid_keylength(length))
      throw Invalid_Key_Length(name(), length);
   key_schedule(key, length);
   }

}
/*
* OctetString
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Create an OctetString from RNG output
*/
OctetString::OctetString(RandomNumberGenerator& rng,
                         size_t len)
   {
   m_data = rng.random_vec(len);
   }

/*
* Create an OctetString from a hex string
*/
OctetString::OctetString(const std::string& hex_string)
   {
   m_data.resize(1 + hex_string.length() / 2);
   m_data.resize(hex_decode(m_data.data(), hex_string));
   }

/*
* Create an OctetString from a byte string
*/
OctetString::OctetString(const uint8_t in[], size_t n)
   {
   m_data.assign(in, in + n);
   }

/*
* Set the parity of each key byte to odd
*/
void OctetString::set_odd_parity()
   {
   const uint8_t ODD_PARITY[256] = {
      0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x07, 0x07, 0x08, 0x08, 0x0B, 0x0B,
      0x0D, 0x0D, 0x0E, 0x0E, 0x10, 0x10, 0x13, 0x13, 0x15, 0x15, 0x16, 0x16,
      0x19, 0x19, 0x1A, 0x1A, 0x1C, 0x1C, 0x1F, 0x1F, 0x20, 0x20, 0x23, 0x23,
      0x25, 0x25, 0x26, 0x26, 0x29, 0x29, 0x2A, 0x2A, 0x2C, 0x2C, 0x2F, 0x2F,
      0x31, 0x31, 0x32, 0x32, 0x34, 0x34, 0x37, 0x37, 0x38, 0x38, 0x3B, 0x3B,
      0x3D, 0x3D, 0x3E, 0x3E, 0x40, 0x40, 0x43, 0x43, 0x45, 0x45, 0x46, 0x46,
      0x49, 0x49, 0x4A, 0x4A, 0x4C, 0x4C, 0x4F, 0x4F, 0x51, 0x51, 0x52, 0x52,
      0x54, 0x54, 0x57, 0x57, 0x58, 0x58, 0x5B, 0x5B, 0x5D, 0x5D, 0x5E, 0x5E,
      0x61, 0x61, 0x62, 0x62, 0x64, 0x64, 0x67, 0x67, 0x68, 0x68, 0x6B, 0x6B,
      0x6D, 0x6D, 0x6E, 0x6E, 0x70, 0x70, 0x73, 0x73, 0x75, 0x75, 0x76, 0x76,
      0x79, 0x79, 0x7A, 0x7A, 0x7C, 0x7C, 0x7F, 0x7F, 0x80, 0x80, 0x83, 0x83,
      0x85, 0x85, 0x86, 0x86, 0x89, 0x89, 0x8A, 0x8A, 0x8C, 0x8C, 0x8F, 0x8F,
      0x91, 0x91, 0x92, 0x92, 0x94, 0x94, 0x97, 0x97, 0x98, 0x98, 0x9B, 0x9B,
      0x9D, 0x9D, 0x9E, 0x9E, 0xA1, 0xA1, 0xA2, 0xA2, 0xA4, 0xA4, 0xA7, 0xA7,
      0xA8, 0xA8, 0xAB, 0xAB, 0xAD, 0xAD, 0xAE, 0xAE, 0xB0, 0xB0, 0xB3, 0xB3,
      0xB5, 0xB5, 0xB6, 0xB6, 0xB9, 0xB9, 0xBA, 0xBA, 0xBC, 0xBC, 0xBF, 0xBF,
      0xC1, 0xC1, 0xC2, 0xC2, 0xC4, 0xC4, 0xC7, 0xC7, 0xC8, 0xC8, 0xCB, 0xCB,
      0xCD, 0xCD, 0xCE, 0xCE, 0xD0, 0xD0, 0xD3, 0xD3, 0xD5, 0xD5, 0xD6, 0xD6,
      0xD9, 0xD9, 0xDA, 0xDA, 0xDC, 0xDC, 0xDF, 0xDF, 0xE0, 0xE0, 0xE3, 0xE3,
      0xE5, 0xE5, 0xE6, 0xE6, 0xE9, 0xE9, 0xEA, 0xEA, 0xEC, 0xEC, 0xEF, 0xEF,
      0xF1, 0xF1, 0xF2, 0xF2, 0xF4, 0xF4, 0xF7, 0xF7, 0xF8, 0xF8, 0xFB, 0xFB,
      0xFD, 0xFD, 0xFE, 0xFE };

   for(size_t j = 0; j != m_data.size(); ++j)
      m_data[j] = ODD_PARITY[m_data[j]];
   }

/*
* Hex encode an OctetString
*/
std::string OctetString::as_string() const
   {
   return hex_encode(m_data.data(), m_data.size());
   }

/*
* XOR Operation for OctetStrings
*/
OctetString& OctetString::operator^=(const OctetString& k)
   {
   if(&k == this) { zeroise(m_data); return (*this); }
   xor_buf(m_data.data(), k.begin(), std::min(length(), k.length()));
   return (*this);
   }

/*
* Equality Operation for OctetStrings
*/
bool operator==(const OctetString& s1, const OctetString& s2)
   {
   return (s1.bits_of() == s2.bits_of());
   }

/*
* Unequality Operation for OctetStrings
*/
bool operator!=(const OctetString& s1, const OctetString& s2)
   {
   return !(s1 == s2);
   }

/*
* Append Operation for OctetStrings
*/
OctetString operator+(const OctetString& k1, const OctetString& k2)
   {
   secure_vector<uint8_t> out;
   out += k1.bits_of();
   out += k2.bits_of();
   return OctetString(out);
   }

/*
* XOR Operation for OctetStrings
*/
OctetString operator^(const OctetString& k1, const OctetString& k2)
   {
   secure_vector<uint8_t> out(std::max(k1.length(), k2.length()));

   copy_mem(out.data(), k1.begin(), k1.length());
   xor_buf(out.data(), k2.begin(), k2.length());
   return OctetString(out);
   }

}
/*
* Blake2b
* (C) 2016 cynecx
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

enum blake2b_constant {
  BLAKE2B_BLOCKBYTES = 128,
  BLAKE2B_IVU64COUNT = 8
};

const uint64_t blake2b_IV[BLAKE2B_IVU64COUNT] = {
   0x6a09e667f3bcc908, 0xbb67ae8584caa73b,
   0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
   0x510e527fade682d1, 0x9b05688c2b3e6c1f,
   0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};

}

Blake2b::Blake2b(size_t output_bits) :
   m_output_bits(output_bits),
   m_buffer(BLAKE2B_BLOCKBYTES),
   m_bufpos(0),
   m_H(BLAKE2B_IVU64COUNT)
   {
   if(output_bits == 0 || output_bits > 512 || output_bits % 8 != 0)
      {
      throw Invalid_Argument("Bad output bits size for Blake2b");
      }

   state_init();
   }

void Blake2b::state_init()
   {
   copy_mem(m_H.data(), blake2b_IV, BLAKE2B_IVU64COUNT);
   m_H[0] ^= 0x01010000 ^ static_cast<uint8_t>(output_length());
   m_T[0] = m_T[1] = 0;
   m_F[0] = m_F[1] = 0;
   }

namespace {

inline void G(uint64_t& a, uint64_t& b, uint64_t& c, uint64_t& d,
              uint64_t M0, uint64_t M1)
   {
   a = a + b + M0;
   d = rotr<32>(d ^ a);
   c = c + d;
   b = rotr<24>(b ^ c);
   a = a + b + M1;
   d = rotr<16>(d ^ a);
   c = c + d;
   b = rotr<63>(b ^ c);
   }

template<size_t i0, size_t i1, size_t i2, size_t i3, size_t i4, size_t i5, size_t i6, size_t i7,
         size_t i8, size_t i9, size_t iA, size_t iB, size_t iC, size_t iD, size_t iE, size_t iF>
inline void ROUND(uint64_t* v, const uint64_t* M)
   {
   G(v[ 0], v[ 4], v[ 8], v[12], M[i0], M[i1]);
   G(v[ 1], v[ 5], v[ 9], v[13], M[i2], M[i3]);
   G(v[ 2], v[ 6], v[10], v[14], M[i4], M[i5]);
   G(v[ 3], v[ 7], v[11], v[15], M[i6], M[i7]);
   G(v[ 0], v[ 5], v[10], v[15], M[i8], M[i9]);
   G(v[ 1], v[ 6], v[11], v[12], M[iA], M[iB]);
   G(v[ 2], v[ 7], v[ 8], v[13], M[iC], M[iD]);
   G(v[ 3], v[ 4], v[ 9], v[14], M[iE], M[iF]);
   }


}

void Blake2b::compress(const uint8_t* input, size_t blocks, uint64_t increment)
   {
   for(size_t b = 0; b != blocks; ++b)
      {
      m_T[0] += increment;
      if(m_T[0] < increment)
         {
         m_T[1]++;
         }

      uint64_t M[16];
      uint64_t v[16];
      load_le(M, input, 16);

      input += BLAKE2B_BLOCKBYTES;

      for(size_t i = 0; i < 8; i++)
         v[i] = m_H[i];
      for(size_t i = 0; i != 8; ++i)
         v[i + 8] = blake2b_IV[i];

      v[12] ^= m_T[0];
      v[13] ^= m_T[1];
      v[14] ^= m_F[0];
      v[15] ^= m_F[1];

      ROUND< 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15>(v, M);
      ROUND<14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3>(v, M);
      ROUND<11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4>(v, M);
      ROUND< 7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8>(v, M);
      ROUND< 9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13>(v, M);
      ROUND< 2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9>(v, M);
      ROUND<12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11>(v, M);
      ROUND<13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10>(v, M);
      ROUND< 6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5>(v, M);
      ROUND<10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13,  0>(v, M);
      ROUND< 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15>(v, M);
      ROUND<14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3>(v, M);

      for(size_t i = 0; i < 8; i++)
         {
         m_H[i] ^= v[i] ^ v[i + 8];
         }
      }
   }

void Blake2b::add_data(const uint8_t input[], size_t length)
   {
   if(length == 0)
      return;

   if(m_bufpos > 0)
      {
      if(m_bufpos < BLAKE2B_BLOCKBYTES)
         {
         const size_t take = std::min(BLAKE2B_BLOCKBYTES - m_bufpos, length);
         copy_mem(&m_buffer[m_bufpos], input, take);
         m_bufpos += take;
         length -= take;
         input += take;
         }

      if(m_bufpos == m_buffer.size() && length > 0)
         {
         compress(m_buffer.data(), 1, BLAKE2B_BLOCKBYTES);
         m_bufpos = 0;
         }
      }

   if(length > BLAKE2B_BLOCKBYTES)
      {
      const size_t full_blocks = ((length-1) / BLAKE2B_BLOCKBYTES);
      compress(input, full_blocks, BLAKE2B_BLOCKBYTES);

      input += full_blocks * BLAKE2B_BLOCKBYTES;
      length -= full_blocks * BLAKE2B_BLOCKBYTES;
      }

   if(length > 0)
      {
      copy_mem(&m_buffer[m_bufpos], input, length);
      m_bufpos += length;
      }
   }

void Blake2b::final_result(uint8_t output[])
   {
   if(m_bufpos != BLAKE2B_BLOCKBYTES)
      clear_mem(&m_buffer[m_bufpos], BLAKE2B_BLOCKBYTES - m_bufpos);
   m_F[0] = 0xFFFFFFFFFFFFFFFF;
   compress(m_buffer.data(), 1, m_bufpos);
   copy_out_vec_le(output, output_length(), m_H);
   clear();
   }

std::string Blake2b::name() const
   {
   return "Blake2b(" + std::to_string(m_output_bits) + ")";
   }

HashFunction* Blake2b::clone() const
   {
   return new Blake2b(m_output_bits);
   }

std::unique_ptr<HashFunction> Blake2b::copy_state() const
   {
   return std::unique_ptr<HashFunction>(new Blake2b(*this));
   }

void Blake2b::clear()
   {
   zeroise(m_H);
   zeroise(m_buffer);
   m_bufpos = 0;
   state_init();
   }

}
/*
* ChaCha
* (C) 2014,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

#define CHACHA_QUARTER_ROUND(a, b, c, d) \
      do {                               \
      a += b; d ^= a; d = rotl<16>(d);   \
      c += d; b ^= c; b = rotl<12>(b);   \
      a += b; d ^= a; d = rotl<8>(d);    \
      c += d; b ^= c; b = rotl<7>(b);    \
      } while(0)

/*
* Generate HChaCha cipher stream (for XChaCha IV setup)
*/
void hchacha(uint32_t output[8], const uint32_t input[16], size_t rounds)
   {
   BOTAN_ASSERT(rounds % 2 == 0, "Valid rounds");

   uint32_t x00 = input[ 0], x01 = input[ 1], x02 = input[ 2], x03 = input[ 3],
            x04 = input[ 4], x05 = input[ 5], x06 = input[ 6], x07 = input[ 7],
            x08 = input[ 8], x09 = input[ 9], x10 = input[10], x11 = input[11],
            x12 = input[12], x13 = input[13], x14 = input[14], x15 = input[15];

   for(size_t i = 0; i != rounds / 2; ++i)
      {
      CHACHA_QUARTER_ROUND(x00, x04, x08, x12);
      CHACHA_QUARTER_ROUND(x01, x05, x09, x13);
      CHACHA_QUARTER_ROUND(x02, x06, x10, x14);
      CHACHA_QUARTER_ROUND(x03, x07, x11, x15);

      CHACHA_QUARTER_ROUND(x00, x05, x10, x15);
      CHACHA_QUARTER_ROUND(x01, x06, x11, x12);
      CHACHA_QUARTER_ROUND(x02, x07, x08, x13);
      CHACHA_QUARTER_ROUND(x03, x04, x09, x14);
      }

   output[0] = x00;
   output[1] = x01;
   output[2] = x02;
   output[3] = x03;
   output[4] = x12;
   output[5] = x13;
   output[6] = x14;
   output[7] = x15;
   }

}

ChaCha::ChaCha(size_t rounds) : m_rounds(rounds)
   {
   BOTAN_ARG_CHECK(m_rounds == 8 || m_rounds == 12 || m_rounds == 20,
                   "ChaCha only supports 8, 12 or 20 rounds");
   }

std::string ChaCha::provider() const
   {
#if defined(BOTAN_HAS_CHACHA_AVX2)
   if(CPUID::has_avx2())
      {
      return "avx2";
      }
#endif

#if defined(BOTAN_HAS_CHACHA_SIMD32)
   if(CPUID::has_simd_32())
      {
      return "simd32";
      }
#endif

   return "base";
   }

//static
void ChaCha::chacha_x8(uint8_t output[64*8], uint32_t input[16], size_t rounds)
   {
   BOTAN_ASSERT(rounds % 2 == 0, "Valid rounds");

#if defined(BOTAN_HAS_CHACHA_AVX2)
   if(CPUID::has_avx2())
      {
      return ChaCha::chacha_avx2_x8(output, input, rounds);
      }
#endif

#if defined(BOTAN_HAS_CHACHA_SIMD32)
   if(CPUID::has_simd_32())
      {
      ChaCha::chacha_simd32_x4(output, input, rounds);
      ChaCha::chacha_simd32_x4(output + 4*64, input, rounds);
      return;
      }
#endif

   // TODO interleave rounds
   for(size_t i = 0; i != 8; ++i)
      {
      uint32_t x00 = input[ 0], x01 = input[ 1], x02 = input[ 2], x03 = input[ 3],
               x04 = input[ 4], x05 = input[ 5], x06 = input[ 6], x07 = input[ 7],
               x08 = input[ 8], x09 = input[ 9], x10 = input[10], x11 = input[11],
               x12 = input[12], x13 = input[13], x14 = input[14], x15 = input[15];

      for(size_t r = 0; r != rounds / 2; ++r)
         {
         CHACHA_QUARTER_ROUND(x00, x04, x08, x12);
         CHACHA_QUARTER_ROUND(x01, x05, x09, x13);
         CHACHA_QUARTER_ROUND(x02, x06, x10, x14);
         CHACHA_QUARTER_ROUND(x03, x07, x11, x15);

         CHACHA_QUARTER_ROUND(x00, x05, x10, x15);
         CHACHA_QUARTER_ROUND(x01, x06, x11, x12);
         CHACHA_QUARTER_ROUND(x02, x07, x08, x13);
         CHACHA_QUARTER_ROUND(x03, x04, x09, x14);
         }

      x00 += input[0];
      x01 += input[1];
      x02 += input[2];
      x03 += input[3];
      x04 += input[4];
      x05 += input[5];
      x06 += input[6];
      x07 += input[7];
      x08 += input[8];
      x09 += input[9];
      x10 += input[10];
      x11 += input[11];
      x12 += input[12];
      x13 += input[13];
      x14 += input[14];
      x15 += input[15];

      store_le(x00, output + 64 * i + 4 *  0);
      store_le(x01, output + 64 * i + 4 *  1);
      store_le(x02, output + 64 * i + 4 *  2);
      store_le(x03, output + 64 * i + 4 *  3);
      store_le(x04, output + 64 * i + 4 *  4);
      store_le(x05, output + 64 * i + 4 *  5);
      store_le(x06, output + 64 * i + 4 *  6);
      store_le(x07, output + 64 * i + 4 *  7);
      store_le(x08, output + 64 * i + 4 *  8);
      store_le(x09, output + 64 * i + 4 *  9);
      store_le(x10, output + 64 * i + 4 * 10);
      store_le(x11, output + 64 * i + 4 * 11);
      store_le(x12, output + 64 * i + 4 * 12);
      store_le(x13, output + 64 * i + 4 * 13);
      store_le(x14, output + 64 * i + 4 * 14);
      store_le(x15, output + 64 * i + 4 * 15);

      input[12]++;
      input[13] += (input[12] == 0);
      }
   }

#undef CHACHA_QUARTER_ROUND

/*
* Combine cipher stream with message
*/
void ChaCha::cipher(const uint8_t in[], uint8_t out[], size_t length)
   {
   verify_key_set(m_state.empty() == false);

   while(length >= m_buffer.size() - m_position)
      {
      const size_t available = m_buffer.size() - m_position;

      xor_buf(out, in, &m_buffer[m_position], available);
      chacha_x8(m_buffer.data(), m_state.data(), m_rounds);

      length -= available;
      in += available;
      out += available;
      m_position = 0;
      }

   xor_buf(out, in, &m_buffer[m_position], length);

   m_position += length;
   }

void ChaCha::write_keystream(uint8_t out[], size_t length)
   {
   verify_key_set(m_state.empty() == false);

   while(length >= m_buffer.size() - m_position)
      {
      const size_t available = m_buffer.size() - m_position;

      copy_mem(out, &m_buffer[m_position], available);
      chacha_x8(m_buffer.data(), m_state.data(), m_rounds);

      length -= available;
      out += available;
      m_position = 0;
      }

   copy_mem(out, &m_buffer[m_position], length);

   m_position += length;
   }

void ChaCha::initialize_state()
   {
   static const uint32_t TAU[] =
      { 0x61707865, 0x3120646e, 0x79622d36, 0x6b206574 };

   static const uint32_t SIGMA[] =
      { 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574 };

   m_state[4] = m_key[0];
   m_state[5] = m_key[1];
   m_state[6] = m_key[2];
   m_state[7] = m_key[3];

   if(m_key.size() == 4)
      {
      m_state[0] = TAU[0];
      m_state[1] = TAU[1];
      m_state[2] = TAU[2];
      m_state[3] = TAU[3];

      m_state[8] = m_key[0];
      m_state[9] = m_key[1];
      m_state[10] = m_key[2];
      m_state[11] = m_key[3];
      }
   else
      {
      m_state[0] = SIGMA[0];
      m_state[1] = SIGMA[1];
      m_state[2] = SIGMA[2];
      m_state[3] = SIGMA[3];

      m_state[8] = m_key[4];
      m_state[9] = m_key[5];
      m_state[10] = m_key[6];
      m_state[11] = m_key[7];
      }

   m_state[12] = 0;
   m_state[13] = 0;
   m_state[14] = 0;
   m_state[15] = 0;

   m_position = 0;
   }

/*
* ChaCha Key Schedule
*/
void ChaCha::key_schedule(const uint8_t key[], size_t length)
   {
   m_key.resize(length / 4);
   load_le<uint32_t>(m_key.data(), key, m_key.size());

   m_state.resize(16);

   const size_t chacha_parallelism = 8; // chacha_x8
   const size_t chacha_block = 64;
   m_buffer.resize(chacha_parallelism * chacha_block);

   set_iv(nullptr, 0);
   }

size_t ChaCha::default_iv_length() const
   {
   return 24;
   }

Key_Length_Specification ChaCha::key_spec() const
   {
   return Key_Length_Specification(16, 32, 16);
   }

StreamCipher* ChaCha::clone() const
   {
   return new ChaCha(m_rounds);
   }

bool ChaCha::valid_iv_length(size_t iv_len) const
   {
   return (iv_len == 0 || iv_len == 8 || iv_len == 12 || iv_len == 24);
   }

void ChaCha::set_iv(const uint8_t iv[], size_t length)
   {
   verify_key_set(m_state.empty() == false);

   if(!valid_iv_length(length))
      throw Invalid_IV_Length(name(), length);

   initialize_state();

   if(length == 0)
      {
      // Treat zero length IV same as an all-zero IV
      m_state[14] = 0;
      m_state[15] = 0;
      }
   else if(length == 8)
      {
      m_state[14] = load_le<uint32_t>(iv, 0);
      m_state[15] = load_le<uint32_t>(iv, 1);
      }
   else if(length == 12)
      {
      m_state[13] = load_le<uint32_t>(iv, 0);
      m_state[14] = load_le<uint32_t>(iv, 1);
      m_state[15] = load_le<uint32_t>(iv, 2);
      }
   else if(length == 24)
      {
      m_state[12] = load_le<uint32_t>(iv, 0);
      m_state[13] = load_le<uint32_t>(iv, 1);
      m_state[14] = load_le<uint32_t>(iv, 2);
      m_state[15] = load_le<uint32_t>(iv, 3);

      secure_vector<uint32_t> hc(8);
      hchacha(hc.data(), m_state.data(), m_rounds);

      m_state[ 4] = hc[0];
      m_state[ 5] = hc[1];
      m_state[ 6] = hc[2];
      m_state[ 7] = hc[3];
      m_state[ 8] = hc[4];
      m_state[ 9] = hc[5];
      m_state[10] = hc[6];
      m_state[11] = hc[7];
      m_state[12] = 0;
      m_state[13] = 0;
      m_state[14] = load_le<uint32_t>(iv, 4);
      m_state[15] = load_le<uint32_t>(iv, 5);
      }

   chacha_x8(m_buffer.data(), m_state.data(), m_rounds);
   m_position = 0;
   }

void ChaCha::clear()
   {
   zap(m_key);
   zap(m_state);
   zap(m_buffer);
   m_position = 0;
   }

std::string ChaCha::name() const
   {
   return "ChaCha(" + std::to_string(m_rounds) + ")";
   }

void ChaCha::seek(uint64_t offset)
   {
   verify_key_set(m_state.empty() == false);

   // Find the block offset
   const uint64_t counter = offset / 64;

   uint8_t out[8];

   store_le(counter, out);

   m_state[12] = load_le<uint32_t>(out, 0);
   m_state[13] += load_le<uint32_t>(out, 1);

   chacha_x8(m_buffer.data(), m_state.data(), m_rounds);
   m_position = offset % 64;
   }
}
/*
* ChaCha20Poly1305 AEAD
* (C) 2014,2016,2018 Jack Lloyd
* (C) 2016 Daniel Neus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

ChaCha20Poly1305_Mode::ChaCha20Poly1305_Mode() :
   m_chacha(StreamCipher::create("ChaCha")),
   m_poly1305(MessageAuthenticationCode::create("Poly1305"))
   {
   if(!m_chacha || !m_poly1305)
      throw Algorithm_Not_Found("ChaCha20Poly1305");
   }

bool ChaCha20Poly1305_Mode::valid_nonce_length(size_t n) const
   {
   return (n == 8 || n == 12 || n == 24);
   }

void ChaCha20Poly1305_Mode::clear()
   {
   m_chacha->clear();
   m_poly1305->clear();
   reset();
   }

void ChaCha20Poly1305_Mode::reset()
   {
   m_ad.clear();
   m_ctext_len = 0;
   m_nonce_len = 0;
   }

void ChaCha20Poly1305_Mode::key_schedule(const uint8_t key[], size_t length)
   {
   m_chacha->set_key(key, length);
   }

void ChaCha20Poly1305_Mode::set_associated_data(const uint8_t ad[], size_t length)
   {
   if(m_ctext_len > 0 || m_nonce_len > 0)
      throw Invalid_State("Cannot set AD for ChaCha20Poly1305 while processing a message");
   m_ad.assign(ad, ad + length);
   }

void ChaCha20Poly1305_Mode::update_len(size_t len)
   {
   uint8_t len8[8] = { 0 };
   store_le(static_cast<uint64_t>(len), len8);
   m_poly1305->update(len8, 8);
   }

void ChaCha20Poly1305_Mode::start_msg(const uint8_t nonce[], size_t nonce_len)
   {
   if(!valid_nonce_length(nonce_len))
      throw Invalid_IV_Length(name(), nonce_len);

   m_ctext_len = 0;
   m_nonce_len = nonce_len;

   m_chacha->set_iv(nonce, nonce_len);

   secure_vector<uint8_t> first_block(64);
   m_chacha->write_keystream(first_block.data(), first_block.size());

   m_poly1305->set_key(first_block.data(), 32);
   // Remainder of first block is discarded

   m_poly1305->update(m_ad);

   if(cfrg_version())
      {
      if(m_ad.size() % 16)
         {
         const uint8_t zeros[16] = { 0 };
         m_poly1305->update(zeros, 16 - m_ad.size() % 16);
         }
      }
   else
      {
      update_len(m_ad.size());
      }
   }

size_t ChaCha20Poly1305_Encryption::process(uint8_t buf[], size_t sz)
   {
   m_chacha->cipher1(buf, sz);
   m_poly1305->update(buf, sz); // poly1305 of ciphertext
   m_ctext_len += sz;
   return sz;
   }

void ChaCha20Poly1305_Encryption::finish(secure_vector<uint8_t>& buffer, size_t offset)
   {
   update(buffer, offset);
   if(cfrg_version())
      {
      if(m_ctext_len % 16)
         {
         const uint8_t zeros[16] = { 0 };
         m_poly1305->update(zeros, 16 - m_ctext_len % 16);
         }
      update_len(m_ad.size());
      }
   update_len(m_ctext_len);

   const secure_vector<uint8_t> mac = m_poly1305->final();
   buffer += std::make_pair(mac.data(), tag_size());
   m_ctext_len = 0;
   m_nonce_len = 0;
   }

size_t ChaCha20Poly1305_Decryption::process(uint8_t buf[], size_t sz)
   {
   m_poly1305->update(buf, sz); // poly1305 of ciphertext
   m_chacha->cipher1(buf, sz);
   m_ctext_len += sz;
   return sz;
   }

void ChaCha20Poly1305_Decryption::finish(secure_vector<uint8_t>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   uint8_t* buf = buffer.data() + offset;

   BOTAN_ASSERT(sz >= tag_size(), "Have the tag as part of final input");

   const size_t remaining = sz - tag_size();

   if(remaining)
      {
      m_poly1305->update(buf, remaining); // poly1305 of ciphertext
      m_chacha->cipher1(buf, remaining);
      m_ctext_len += remaining;
      }

   if(cfrg_version())
      {
      if(m_ctext_len % 16)
         {
         const uint8_t zeros[16] = { 0 };
         m_poly1305->update(zeros, 16 - m_ctext_len % 16);
         }
      update_len(m_ad.size());
      }

   update_len(m_ctext_len);
   const secure_vector<uint8_t> mac = m_poly1305->final();

   const uint8_t* included_tag = &buf[remaining];

   m_ctext_len = 0;
   m_nonce_len = 0;

   if(!constant_time_compare(mac.data(), included_tag, tag_size()))
      throw Integrity_Failure("ChaCha20Poly1305 tag check failed");
   buffer.resize(offset + remaining);
   }

}
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

//static
BOTAN_FUNC_ISA("avx2")
void ChaCha::chacha_avx2_x8(uint8_t output[64*8], uint32_t state[16], size_t rounds)
   {
   SIMD_8x32::reset_registers();

   BOTAN_ASSERT(rounds % 2 == 0, "Valid rounds");
   const SIMD_8x32 CTR0 = SIMD_8x32(0, 1, 2, 3, 4, 5, 6, 7);

   const uint32_t C = 0xFFFFFFFF - state[12];
   const SIMD_8x32 CTR1 = SIMD_8x32(0, C < 1, C < 2, C < 3, C < 4, C < 5, C < 6, C < 7);

   SIMD_8x32 R00 = SIMD_8x32::splat(state[ 0]);
   SIMD_8x32 R01 = SIMD_8x32::splat(state[ 1]);
   SIMD_8x32 R02 = SIMD_8x32::splat(state[ 2]);
   SIMD_8x32 R03 = SIMD_8x32::splat(state[ 3]);
   SIMD_8x32 R04 = SIMD_8x32::splat(state[ 4]);
   SIMD_8x32 R05 = SIMD_8x32::splat(state[ 5]);
   SIMD_8x32 R06 = SIMD_8x32::splat(state[ 6]);
   SIMD_8x32 R07 = SIMD_8x32::splat(state[ 7]);
   SIMD_8x32 R08 = SIMD_8x32::splat(state[ 8]);
   SIMD_8x32 R09 = SIMD_8x32::splat(state[ 9]);
   SIMD_8x32 R10 = SIMD_8x32::splat(state[10]);
   SIMD_8x32 R11 = SIMD_8x32::splat(state[11]);
   SIMD_8x32 R12 = SIMD_8x32::splat(state[12]) + CTR0;
   SIMD_8x32 R13 = SIMD_8x32::splat(state[13]) + CTR1;
   SIMD_8x32 R14 = SIMD_8x32::splat(state[14]);
   SIMD_8x32 R15 = SIMD_8x32::splat(state[15]);

   for(size_t r = 0; r != rounds / 2; ++r)
      {
      R00 += R04;
      R01 += R05;
      R02 += R06;
      R03 += R07;

      R12 ^= R00;
      R13 ^= R01;
      R14 ^= R02;
      R15 ^= R03;

      R12 = R12.rotl<16>();
      R13 = R13.rotl<16>();
      R14 = R14.rotl<16>();
      R15 = R15.rotl<16>();

      R08 += R12;
      R09 += R13;
      R10 += R14;
      R11 += R15;

      R04 ^= R08;
      R05 ^= R09;
      R06 ^= R10;
      R07 ^= R11;

      R04 = R04.rotl<12>();
      R05 = R05.rotl<12>();
      R06 = R06.rotl<12>();
      R07 = R07.rotl<12>();

      R00 += R04;
      R01 += R05;
      R02 += R06;
      R03 += R07;

      R12 ^= R00;
      R13 ^= R01;
      R14 ^= R02;
      R15 ^= R03;

      R12 = R12.rotl<8>();
      R13 = R13.rotl<8>();
      R14 = R14.rotl<8>();
      R15 = R15.rotl<8>();

      R08 += R12;
      R09 += R13;
      R10 += R14;
      R11 += R15;

      R04 ^= R08;
      R05 ^= R09;
      R06 ^= R10;
      R07 ^= R11;

      R04 = R04.rotl<7>();
      R05 = R05.rotl<7>();
      R06 = R06.rotl<7>();
      R07 = R07.rotl<7>();

      R00 += R05;
      R01 += R06;
      R02 += R07;
      R03 += R04;

      R15 ^= R00;
      R12 ^= R01;
      R13 ^= R02;
      R14 ^= R03;

      R15 = R15.rotl<16>();
      R12 = R12.rotl<16>();
      R13 = R13.rotl<16>();
      R14 = R14.rotl<16>();

      R10 += R15;
      R11 += R12;
      R08 += R13;
      R09 += R14;

      R05 ^= R10;
      R06 ^= R11;
      R07 ^= R08;
      R04 ^= R09;

      R05 = R05.rotl<12>();
      R06 = R06.rotl<12>();
      R07 = R07.rotl<12>();
      R04 = R04.rotl<12>();

      R00 += R05;
      R01 += R06;
      R02 += R07;
      R03 += R04;

      R15 ^= R00;
      R12 ^= R01;
      R13 ^= R02;
      R14 ^= R03;

      R15 = R15.rotl<8>();
      R12 = R12.rotl<8>();
      R13 = R13.rotl<8>();
      R14 = R14.rotl<8>();

      R10 += R15;
      R11 += R12;
      R08 += R13;
      R09 += R14;

      R05 ^= R10;
      R06 ^= R11;
      R07 ^= R08;
      R04 ^= R09;

      R05 = R05.rotl<7>();
      R06 = R06.rotl<7>();
      R07 = R07.rotl<7>();
      R04 = R04.rotl<7>();
      }

   R00 += SIMD_8x32::splat(state[0]);
   R01 += SIMD_8x32::splat(state[1]);
   R02 += SIMD_8x32::splat(state[2]);
   R03 += SIMD_8x32::splat(state[3]);
   R04 += SIMD_8x32::splat(state[4]);
   R05 += SIMD_8x32::splat(state[5]);
   R06 += SIMD_8x32::splat(state[6]);
   R07 += SIMD_8x32::splat(state[7]);
   R08 += SIMD_8x32::splat(state[8]);
   R09 += SIMD_8x32::splat(state[9]);
   R10 += SIMD_8x32::splat(state[10]);
   R11 += SIMD_8x32::splat(state[11]);
   R12 += SIMD_8x32::splat(state[12]) + CTR0;
   R13 += SIMD_8x32::splat(state[13]) + CTR1;
   R14 += SIMD_8x32::splat(state[14]);
   R15 += SIMD_8x32::splat(state[15]);

   SIMD_8x32::transpose(R00, R01, R02, R03);
   SIMD_8x32::transpose(R04, R05, R06, R07);
   SIMD_8x32::transpose(R08, R09, R10, R11);
   SIMD_8x32::transpose(R12, R13, R14, R15);

   __m256i* output_mm = reinterpret_cast<__m256i*>(output);

   _mm256_storeu_si256(output_mm     , _mm256_permute2x128_si256(R00.handle(), R04.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  1, _mm256_permute2x128_si256(R08.handle(), R12.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  2, _mm256_permute2x128_si256(R01.handle(), R05.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  3, _mm256_permute2x128_si256(R09.handle(), R13.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  4, _mm256_permute2x128_si256(R02.handle(), R06.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  5, _mm256_permute2x128_si256(R10.handle(), R14.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  6, _mm256_permute2x128_si256(R03.handle(), R07.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  7, _mm256_permute2x128_si256(R11.handle(), R15.handle(), 0 + (2 << 4)));

   _mm256_storeu_si256(output_mm +  8, _mm256_permute2x128_si256(R00.handle(), R04.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm +  9, _mm256_permute2x128_si256(R08.handle(), R12.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 10, _mm256_permute2x128_si256(R01.handle(), R05.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 11, _mm256_permute2x128_si256(R09.handle(), R13.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 12, _mm256_permute2x128_si256(R02.handle(), R06.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 13, _mm256_permute2x128_si256(R10.handle(), R14.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 14, _mm256_permute2x128_si256(R03.handle(), R07.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 15, _mm256_permute2x128_si256(R11.handle(), R15.handle(), 1 + (3 << 4)));

   SIMD_8x32::zero_registers();

   state[12] += 8;
   if(state[12] < 8)
      state[13]++;
   }
}
/*
* Runtime CPU detection
* (C) 2009,2010,2013,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <ostream>

namespace Botan {

uint64_t CPUID::g_processor_features = 0;
size_t CPUID::g_cache_line_size = BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE;
CPUID::Endian_status CPUID::g_endian_status = ENDIAN_UNKNOWN;

bool CPUID::has_simd_32()
   {
#if defined(BOTAN_TARGET_SUPPORTS_SSE2)
   return CPUID::has_sse2();
#elif defined(BOTAN_TARGET_SUPPORTS_ALTIVEC)
   return CPUID::has_altivec();
#elif defined(BOTAN_TARGET_SUPPORTS_NEON)
   return CPUID::has_neon();
#else
   return true;
#endif
   }

//static
std::string CPUID::to_string()
   {
   std::vector<std::string> flags;

#define CPUID_PRINT(flag) do { if(has_##flag()) { flags.push_back(#flag); } } while(0)

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
   CPUID_PRINT(sse2);
   CPUID_PRINT(ssse3);
   CPUID_PRINT(sse41);
   CPUID_PRINT(sse42);
   CPUID_PRINT(avx2);
   CPUID_PRINT(avx512f);

   CPUID_PRINT(rdtsc);
   CPUID_PRINT(bmi1);
   CPUID_PRINT(bmi2);
   CPUID_PRINT(adx);

   CPUID_PRINT(aes_ni);
   CPUID_PRINT(clmul);
   CPUID_PRINT(rdrand);
   CPUID_PRINT(rdseed);
   CPUID_PRINT(intel_sha);
#endif

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
   CPUID_PRINT(altivec);
   CPUID_PRINT(ppc_crypto);
#endif

#if defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
   CPUID_PRINT(neon);
   CPUID_PRINT(arm_sve);

   CPUID_PRINT(arm_sha1);
   CPUID_PRINT(arm_sha2);
   CPUID_PRINT(arm_aes);
   CPUID_PRINT(arm_pmull);
   CPUID_PRINT(arm_sha2_512);
   CPUID_PRINT(arm_sha3);
   CPUID_PRINT(arm_sm3);
   CPUID_PRINT(arm_sm4);
#endif

#undef CPUID_PRINT

   return string_join(flags, ' ');
   }

//static
void CPUID::print(std::ostream& o)
   {
   o << "CPUID flags: " << CPUID::to_string() << "\n";
   }

//static
void CPUID::initialize()
   {
   g_processor_features = 0;

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY) || \
    defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY) || \
    defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

   g_processor_features = CPUID::detect_cpu_features(&g_cache_line_size);

#endif

   g_endian_status = runtime_check_endian();
   g_processor_features |= CPUID::CPUID_INITIALIZED_BIT;
   }

//static
CPUID::Endian_status CPUID::runtime_check_endian()
   {
   // Check runtime endian
   const uint32_t endian32 = 0x01234567;
   const uint8_t* e8 = reinterpret_cast<const uint8_t*>(&endian32);

   Endian_status endian = ENDIAN_UNKNOWN;

   if(e8[0] == 0x01 && e8[1] == 0x23 && e8[2] == 0x45 && e8[3] == 0x67)
      {
      endian = ENDIAN_BIG;
      }
   else if(e8[0] == 0x67 && e8[1] == 0x45 && e8[2] == 0x23 && e8[3] == 0x01)
      {
      endian = ENDIAN_LITTLE;
      }
   else
      {
      throw Internal_Error("Unexpected endian at runtime, neither big nor little");
      }

   // If we were compiled with a known endian, verify it matches at runtime
#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
   BOTAN_ASSERT(endian == ENDIAN_LITTLE, "Build and runtime endian match");
#elif defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
   BOTAN_ASSERT(endian == ENDIAN_BIG, "Build and runtime endian match");
#endif

   return endian;
   }

std::vector<Botan::CPUID::CPUID_bits>
CPUID::bit_from_string(const std::string& tok)
   {
#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
   if(tok == "sse2" || tok == "simd")
      return {Botan::CPUID::CPUID_SSE2_BIT};
   if(tok == "ssse3")
      return {Botan::CPUID::CPUID_SSSE3_BIT};
   if(tok == "aesni")
      return {Botan::CPUID::CPUID_AESNI_BIT};
   if(tok == "clmul")
      return {Botan::CPUID::CPUID_CLMUL_BIT};
   if(tok == "avx2")
      return {Botan::CPUID::CPUID_AVX2_BIT};
   if(tok == "sha")
      return {Botan::CPUID::CPUID_SHA_BIT};
   if(tok == "bmi2")
      return {Botan::CPUID::CPUID_BMI2_BIT};
   if(tok == "adx")
      return {Botan::CPUID::CPUID_ADX_BIT};
   if(tok == "intel_sha")
      return {Botan::CPUID::CPUID_SHA_BIT};

#elif defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
   if(tok == "altivec" || tok == "simd")
      return {Botan::CPUID::CPUID_ALTIVEC_BIT};
   if(tok == "ppc_crypto")
      return {Botan::CPUID::CPUID_PPC_CRYPTO_BIT};

#elif defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
   if(tok == "neon" || tok == "simd")
      return {Botan::CPUID::CPUID_ARM_NEON_BIT};
   if(tok == "armv8sha1")
      return {Botan::CPUID::CPUID_ARM_SHA1_BIT};
   if(tok == "armv8sha2")
      return {Botan::CPUID::CPUID_ARM_SHA2_BIT};
   if(tok == "armv8aes")
      return {Botan::CPUID::CPUID_ARM_AES_BIT};
   if(tok == "armv8pmull")
      return {Botan::CPUID::CPUID_ARM_PMULL_BIT};
   if(tok == "armv8sha3")
      return {Botan::CPUID::CPUID_ARM_SHA3_BIT};
   if(tok == "armv8sha2_512")
      return {Botan::CPUID::CPUID_ARM_SHA2_512_BIT};
   if(tok == "armv8sm3")
      return {Botan::CPUID::CPUID_ARM_SM3_BIT};
   if(tok == "armv8sm4")
      return {Botan::CPUID::CPUID_ARM_SM4_BIT};

#else
   BOTAN_UNUSED(tok);
#endif

   return {};
   }

}
/*
* Runtime CPU detection for ARM
* (C) 2009,2010,2013,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL)
  #include <sys/auxv.h>

#elif defined(BOTAN_TARGET_OS_IS_IOS)
  #include <sys/types.h>
  #include <sys/sysctl.h>

#else

#endif

#endif

namespace Botan {

#if defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)

#if defined(BOTAN_TARGET_OS_IS_IOS)

namespace {

uint64_t flags_by_ios_machine_type(const std::string& machine)
   {
   /*
   * This relies on a map of known machine names to features. This
   * will quickly grow out of date as new products are introduced, but
   * is apparently the best we can do for iOS.
   */

   struct version_info {
      std::string name;
      size_t min_version_neon;
      size_t min_version_armv8;
      };

   static const version_info min_versions[] = {
      { "iPhone", 2, 6 },
      { "iPad", 1, 4 },
      { "iPod", 4, 7 },
      { "AppleTV", 2, 5 },
   };

   if(machine.size() < 3)
      return 0;

   auto comma = machine.find(',');

   // Simulator, or something we don't know about
   if(comma == std::string::npos)
      return 0;

   std::string product = machine.substr(0, comma);

   size_t version = 0;
   size_t place = 1;
   while(product.size() > 1 && ::isdigit(product.back()))
      {
      const size_t digit = product.back() - '0';
      version += digit * place;
      place *= 10;
      product.pop_back();
      }

   if(version == 0)
      return 0;

   for(const version_info& info : min_versions)
      {
      if(info.name != product)
         continue;

      if(version >= info.min_version_armv8)
         {
         return CPUID::CPUID_ARM_AES_BIT |
                CPUID::CPUID_ARM_PMULL_BIT |
                CPUID::CPUID_ARM_SHA1_BIT |
                CPUID::CPUID_ARM_SHA2_BIT |
                CPUID::CPUID_ARM_NEON_BIT;
         }

      if(version >= info.min_version_neon)
         return CPUID::CPUID_ARM_NEON_BIT;
      }

   // Some other product we don't know about
   return 0;
   }

}

#endif

uint64_t CPUID::detect_cpu_features(size_t* cache_line_size)
   {
   uint64_t detected_features = 0;

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL)
   /*
   * On systems with getauxval these bits should normally be defined
   * in bits/auxv.h but some buggy? glibc installs seem to miss them.
   * These following values are all fixed, for the Linux ELF format,
   * so we just hardcode them in ARM_hwcap_bit enum.
   */

   enum ARM_hwcap_bit {
#if defined(BOTAN_TARGET_ARCH_IS_ARM32)
      NEON_bit  = (1 << 12),
      AES_bit   = (1 << 0),
      PMULL_bit = (1 << 1),
      SHA1_bit  = (1 << 2),
      SHA2_bit  = (1 << 3),

      ARCH_hwcap_neon   = 16, // AT_HWCAP
      ARCH_hwcap_crypto = 26, // AT_HWCAP2
#elif defined(BOTAN_TARGET_ARCH_IS_ARM64)
      NEON_bit  = (1 << 1),
      AES_bit   = (1 << 3),
      PMULL_bit = (1 << 4),
      SHA1_bit  = (1 << 5),
      SHA2_bit  = (1 << 6),
      SHA3_bit  = (1 << 17),
      SM3_bit  = (1 << 18),
      SM4_bit  = (1 << 19),
      SHA2_512_bit  = (1 << 21),
      SVE_bit = (1 << 22),

      ARCH_hwcap_neon   = 16, // AT_HWCAP
      ARCH_hwcap_crypto = 16, // AT_HWCAP
#endif
   };

#if defined(AT_DCACHEBSIZE)
   const unsigned long dcache_line = ::getauxval(AT_DCACHEBSIZE);

   // plausibility check
   if(dcache_line == 32 || dcache_line == 64 || dcache_line == 128)
      *cache_line_size = static_cast<size_t>(dcache_line);
#endif

   const unsigned long hwcap_neon = ::getauxval(ARM_hwcap_bit::ARCH_hwcap_neon);
   if(hwcap_neon & ARM_hwcap_bit::NEON_bit)
      detected_features |= CPUID::CPUID_ARM_NEON_BIT;

   /*
   On aarch64 this ends up calling getauxval twice with AT_HWCAP
   It doesn't seem worth optimizing this out, since getauxval is
   just reading a field in the ELF header.
   */
   const unsigned long hwcap_crypto = ::getauxval(ARM_hwcap_bit::ARCH_hwcap_crypto);
   if(hwcap_crypto & ARM_hwcap_bit::AES_bit)
      detected_features |= CPUID::CPUID_ARM_AES_BIT;
   if(hwcap_crypto & ARM_hwcap_bit::PMULL_bit)
      detected_features |= CPUID::CPUID_ARM_PMULL_BIT;
   if(hwcap_crypto & ARM_hwcap_bit::SHA1_bit)
      detected_features |= CPUID::CPUID_ARM_SHA1_BIT;
   if(hwcap_crypto & ARM_hwcap_bit::SHA2_bit)
      detected_features |= CPUID::CPUID_ARM_SHA2_BIT;

#if defined(BOTAN_TARGET_ARCH_IS_ARM64)
   if(hwcap_crypto & ARM_hwcap_bit::SHA3_bit)
      detected_features |= CPUID::CPUID_ARM_SHA3_BIT;
   if(hwcap_crypto & ARM_hwcap_bit::SM3_bit)
      detected_features |= CPUID::CPUID_ARM_SM3_BIT;
   if(hwcap_crypto & ARM_hwcap_bit::SM4_bit)
      detected_features |= CPUID::CPUID_ARM_SM4_BIT;
   if(hwcap_crypto & ARM_hwcap_bit::SHA2_512_bit)
      detected_features |= CPUID::CPUID_ARM_SHA2_512_BIT;
   if(hwcap_crypto & ARM_hwcap_bit::SVE_bit)
      detected_features |= CPUID::CPUID_ARM_SVE_BIT;
#endif

#elif defined(BOTAN_TARGET_OS_IS_IOS)

   char machine[64] = { 0 };
   size_t size = sizeof(machine) - 1;
   ::sysctlbyname("hw.machine", machine, &size, nullptr, 0);

   detected_features = flags_by_ios_machine_type(machine);

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_ARM64)

   /*
   No getauxval API available, fall back on probe functions. We only
   bother with Aarch64 here to simplify the code and because going to
   extreme contortions to support detect NEON on devices that probably
   don't support it doesn't seem worthwhile.

   NEON registers v0-v7 are caller saved in Aarch64
   */

   auto neon_probe  = []() -> int { asm("and v0.16b, v0.16b, v0.16b"); return 1; };
   auto aes_probe   = []() -> int { asm(".word 0x4e284800"); return 1; };
   auto pmull_probe = []() -> int { asm(".word 0x0ee0e000"); return 1; };
   auto sha1_probe  = []() -> int { asm(".word 0x5e280800"); return 1; };
   auto sha2_probe  = []() -> int { asm(".word 0x5e282800"); return 1; };

   // Only bother running the crypto detection if we found NEON

   if(OS::run_cpu_instruction_probe(neon_probe) == 1)
      {
      detected_features |= CPUID::CPUID_ARM_NEON_BIT;

      if(OS::run_cpu_instruction_probe(aes_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_AES_BIT;
      if(OS::run_cpu_instruction_probe(pmull_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_PMULL_BIT;
      if(OS::run_cpu_instruction_probe(sha1_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_SHA1_BIT;
      if(OS::run_cpu_instruction_probe(sha2_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_SHA2_BIT;
      }

#endif

   return detected_features;
   }

#endif

}
/*
* Runtime CPU detection for POWER/PowerPC
* (C) 2009,2010,2013,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)

/*
* On Darwin and OpenBSD ppc, use sysctl to detect AltiVec
*/
#if defined(BOTAN_TARGET_OS_IS_DARWIN)
  #include <sys/sysctl.h>
#elif defined(BOTAN_TARGET_OS_IS_OPENBSD)
  #include <sys/param.h>
  #include <sys/sysctl.h>
  #include <machine/cpu.h>
#elif defined(BOTAN_TARGET_OS_HAS_GETAUXVAL)
  #include <sys/auxv.h>
#endif

#endif

namespace Botan {

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)

/*
* PowerPC specific block: check for AltiVec using either
* sysctl or by reading processor version number register.
*/
uint64_t CPUID::detect_cpu_features(size_t* cache_line_size)
   {
   BOTAN_UNUSED(cache_line_size);

#if defined(BOTAN_TARGET_OS_IS_DARWIN) || defined(BOTAN_TARGET_OS_IS_OPENBSD)
   // On Darwin/OS X and OpenBSD, use sysctl

   int sels[2] = {
#if defined(BOTAN_TARGET_OS_IS_OPENBSD)
      CTL_MACHDEP, CPU_ALTIVEC
#else
      CTL_HW, HW_VECTORUNIT
#endif
   };

   int vector_type = 0;
   size_t length = sizeof(vector_type);
   int error = ::sysctl(sels, 2, &vector_type, &length, NULL, 0);

   if(error == 0 && vector_type > 0)
      return CPUID::CPUID_ALTIVEC_BIT;

#elif defined(BOTAN_TARGET_OS_HAS_GETAUXVAL) && defined(BOTAN_TARGET_ARCH_IS_PPC64)

   enum PPC_hwcap_bit {
      ALTIVEC_bit  = (1 << 28),
      CRYPTO_bit   = (1 << 25),

      ARCH_hwcap_altivec = 16, // AT_HWCAP
      ARCH_hwcap_crypto  = 26, // AT_HWCAP2
   };

   uint64_t detected_features = 0;

   const unsigned long hwcap_altivec = ::getauxval(PPC_hwcap_bit::ARCH_hwcap_altivec);
   if(hwcap_altivec & PPC_hwcap_bit::ALTIVEC_bit)
      detected_features |= CPUID::CPUID_ALTIVEC_BIT;

   const unsigned long hwcap_crypto = ::getauxval(PPC_hwcap_bit::ARCH_hwcap_crypto);
   if(hwcap_crypto & PPC_hwcap_bit::CRYPTO_bit)
     detected_features |= CPUID::CPUID_PPC_CRYPTO_BIT;

   return detected_features;

#else

   /*
   On PowerPC, MSR 287 is PVR, the Processor Version Number
   Normally it is only accessible to ring 0, but Linux and NetBSD
   (others, too, maybe?) will trap and emulate it for us.
   */

   int pvr = OS::run_cpu_instruction_probe([]() -> int {
      uint32_t pvr = 0;
      asm volatile("mfspr %0, 287" : "=r" (pvr));
      // Top 16 bits suffice to identify the model
      return static_cast<int>(pvr >> 16);
      });

   if(pvr > 0)
      {
      const uint16_t ALTIVEC_PVR[] = {
         0x003E, // IBM POWER6
         0x003F, // IBM POWER7
         0x004A, // IBM POWER7p
         0x004B, // IBM POWER8E
         0x004C, // IBM POWER8 NVL
         0x004D, // IBM POWER8
         0x004E, // IBM POWER9
         0x000C, // G4-7400
         0x0039, // G5 970
         0x003C, // G5 970FX
         0x0044, // G5 970MP
         0x0070, // Cell PPU
         0, // end
      };

      for(size_t i = 0; ALTIVEC_PVR[i]; ++i)
         {
         if(pvr == ALTIVEC_PVR[i])
            return CPUID::CPUID_ALTIVEC_BIT;
         }

      return 0;
      }

   // TODO try direct instruction probing

#endif

   return 0;
   }

#endif

}
/*
* Runtime CPU detection for x86
* (C) 2009,2010,2013,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

#if defined(BOTAN_BUILD_COMPILER_IS_MSVC)
  #include <intrin.h>
#elif defined(BOTAN_BUILD_COMPILER_IS_INTEL)
  #include <ia32intrin.h>
#elif defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)
  #include <cpuid.h>
#endif

#endif

namespace Botan {

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

uint64_t CPUID::detect_cpu_features(size_t* cache_line_size)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_MSVC)
  #define X86_CPUID(type, out) do { __cpuid((int*)out, type); } while(0)
  #define X86_CPUID_SUBLEVEL(type, level, out) do { __cpuidex((int*)out, type, level); } while(0)

#elif defined(BOTAN_BUILD_COMPILER_IS_INTEL)
  #define X86_CPUID(type, out) do { __cpuid(out, type); } while(0)
  #define X86_CPUID_SUBLEVEL(type, level, out) do { __cpuidex((int*)out, type, level); } while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_X86_64) && defined(BOTAN_USE_GCC_INLINE_ASM)
  #define X86_CPUID(type, out)                                                    \
     asm("cpuid\n\t" : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3]) \
         : "0" (type))

  #define X86_CPUID_SUBLEVEL(type, level, out)                                    \
     asm("cpuid\n\t" : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3]) \
         : "0" (type), "2" (level))

#elif defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)
  #define X86_CPUID(type, out) do { __get_cpuid(type, out, out+1, out+2, out+3); } while(0)

  #define X86_CPUID_SUBLEVEL(type, level, out) \
     do { __cpuid_count(type, level, out[0], out[1], out[2], out[3]); } while(0)
#else
  #warning "No way of calling x86 cpuid instruction for this compiler"
  #define X86_CPUID(type, out) do { clear_mem(out, 4); } while(0)
  #define X86_CPUID_SUBLEVEL(type, level, out) do { clear_mem(out, 4); } while(0)
#endif

   uint64_t features_detected = 0;
   uint32_t cpuid[4] = { 0 };

   // CPUID 0: vendor identification, max sublevel
   X86_CPUID(0, cpuid);

   const uint32_t max_supported_sublevel = cpuid[0];

   const uint32_t INTEL_CPUID[3] = { 0x756E6547, 0x6C65746E, 0x49656E69 };
   const uint32_t AMD_CPUID[3] = { 0x68747541, 0x444D4163, 0x69746E65 };
   const bool is_intel = same_mem(cpuid + 1, INTEL_CPUID, 3);
   const bool is_amd = same_mem(cpuid + 1, AMD_CPUID, 3);

   if(max_supported_sublevel >= 1)
      {
      // CPUID 1: feature bits
      X86_CPUID(1, cpuid);
      const uint64_t flags0 = (static_cast<uint64_t>(cpuid[2]) << 32) | cpuid[3];

      enum x86_CPUID_1_bits : uint64_t {
         RDTSC = (1ULL << 4),
         SSE2 = (1ULL << 26),
         CLMUL = (1ULL << 33),
         SSSE3 = (1ULL << 41),
         SSE41 = (1ULL << 51),
         SSE42 = (1ULL << 52),
         AESNI = (1ULL << 57),
         RDRAND = (1ULL << 62)
      };

      if(flags0 & x86_CPUID_1_bits::RDTSC)
         features_detected |= CPUID::CPUID_RDTSC_BIT;
      if(flags0 & x86_CPUID_1_bits::SSE2)
         features_detected |= CPUID::CPUID_SSE2_BIT;
      if(flags0 & x86_CPUID_1_bits::CLMUL)
         features_detected |= CPUID::CPUID_CLMUL_BIT;
      if(flags0 & x86_CPUID_1_bits::SSSE3)
         features_detected |= CPUID::CPUID_SSSE3_BIT;
      if(flags0 & x86_CPUID_1_bits::SSE41)
         features_detected |= CPUID::CPUID_SSE41_BIT;
      if(flags0 & x86_CPUID_1_bits::SSE42)
         features_detected |= CPUID::CPUID_SSE42_BIT;
      if(flags0 & x86_CPUID_1_bits::AESNI)
         features_detected |= CPUID::CPUID_AESNI_BIT;
      if(flags0 & x86_CPUID_1_bits::RDRAND)
         features_detected |= CPUID::CPUID_RDRAND_BIT;
      }

   if(is_intel)
      {
      // Intel cache line size is in cpuid(1) output
      *cache_line_size = 8 * get_byte(2, cpuid[1]);
      }
   else if(is_amd)
      {
      // AMD puts it in vendor zone
      X86_CPUID(0x80000005, cpuid);
      *cache_line_size = get_byte(3, cpuid[2]);
      }

   if(max_supported_sublevel >= 7)
      {
      clear_mem(cpuid, 4);
      X86_CPUID_SUBLEVEL(7, 0, cpuid);

      enum x86_CPUID_7_bits : uint64_t {
         BMI1 = (1ULL << 3),
         AVX2 = (1ULL << 5),
         BMI2 = (1ULL << 8),
         AVX512F = (1ULL << 16),
         RDSEED = (1ULL << 18),
         ADX = (1ULL << 19),
         SHA = (1ULL << 29),
      };
      uint64_t flags7 = (static_cast<uint64_t>(cpuid[2]) << 32) | cpuid[1];

      if(flags7 & x86_CPUID_7_bits::AVX2)
         features_detected |= CPUID::CPUID_AVX2_BIT;
      if(flags7 & x86_CPUID_7_bits::BMI1)
         {
         features_detected |= CPUID::CPUID_BMI1_BIT;
         /*
         We only set the BMI2 bit if BMI1 is also supported, so BMI2
         code can safely use both extensions. No known processor
         implements BMI2 but not BMI1.
         */
         if(flags7 & x86_CPUID_7_bits::BMI2)
            features_detected |= CPUID::CPUID_BMI2_BIT;
         }

      if(flags7 & x86_CPUID_7_bits::AVX512F)
         features_detected |= CPUID::CPUID_AVX512F_BIT;
      if(flags7 & x86_CPUID_7_bits::RDSEED)
         features_detected |= CPUID::CPUID_RDSEED_BIT;
      if(flags7 & x86_CPUID_7_bits::ADX)
         features_detected |= CPUID::CPUID_ADX_BIT;
      if(flags7 & x86_CPUID_7_bits::SHA)
         features_detected |= CPUID::CPUID_SHA_BIT;
      }

#undef X86_CPUID
#undef X86_CPUID_SUBLEVEL

   /*
   * If we don't have access to CPUID, we can still safely assume that
   * any x86-64 processor has SSE2 and RDTSC
   */
#if defined(BOTAN_TARGET_ARCH_IS_X86_64)
   if(features_detected == 0)
      {
      features_detected |= CPUID::CPUID_SSE2_BIT;
      features_detected |= CPUID::CPUID_RDTSC_BIT;
      }
#endif

   return features_detected;
   }

#endif

}
/*
* Entropy Source Polling
* (C) 2008-2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_SYSTEM_RNG)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDRAND)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDSEED)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_WIN32)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_PROC_WALKER)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_GETENTROPY)
#endif

namespace Botan {

#if defined(BOTAN_HAS_SYSTEM_RNG)

namespace {

class System_RNG_EntropySource final : public Entropy_Source
   {
   public:
      size_t poll(RandomNumberGenerator& rng) override
         {
         const size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS;
         rng.reseed_from_rng(system_rng(), poll_bits);
         return poll_bits;
         }

      std::string name() const override { return "system_rng"; }
   };

}

#endif

std::unique_ptr<Entropy_Source> Entropy_Source::create(const std::string& name)
   {
#if defined(BOTAN_HAS_SYSTEM_RNG)
   if(name == "system_rng" || name == "win32_cryptoapi")
      {
      return std::unique_ptr<Entropy_Source>(new System_RNG_EntropySource);
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDRAND)
   if(name == "rdrand")
      {
      return std::unique_ptr<Entropy_Source>(new Intel_Rdrand);
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDSEED)
   if(name == "rdseed")
      {
      return std::unique_ptr<Entropy_Source>(new Intel_Rdseed);
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_GETENTROPY)
   if(name == "getentropy")
      {
      return std::unique_ptr<Entropy_Source>(new Getentropy);
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM)
   if(name == "dev_random")
      {
      return std::unique_ptr<Entropy_Source>(new Device_EntropySource(BOTAN_SYSTEM_RNG_POLL_DEVICES));
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_PROC_WALKER)
   if(name == "proc_walk" && OS::running_in_privileged_state() == false)
      {
      const std::string root_dir = BOTAN_ENTROPY_PROC_FS_PATH;
      if(!root_dir.empty())
         return std::unique_ptr<Entropy_Source>(new ProcWalking_EntropySource(root_dir));
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_WIN32)
   if(name == "system_stats")
      {
      return std::unique_ptr<Entropy_Source>(new Win32_EntropySource);
      }
#endif

   BOTAN_UNUSED(name);
   return std::unique_ptr<Entropy_Source>();
   }

void Entropy_Sources::add_source(std::unique_ptr<Entropy_Source> src)
   {
   if(src.get())
      {
      m_srcs.push_back(std::move(src));
      }
   }

std::vector<std::string> Entropy_Sources::enabled_sources() const
   {
   std::vector<std::string> sources;
   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      sources.push_back(m_srcs[i]->name());
      }
   return sources;
   }

size_t Entropy_Sources::poll(RandomNumberGenerator& rng,
                             size_t poll_bits,
                             std::chrono::milliseconds timeout)
   {
   typedef std::chrono::system_clock clock;

   auto deadline = clock::now() + timeout;

   size_t bits_collected = 0;

   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      bits_collected += m_srcs[i]->poll(rng);

      if (bits_collected >= poll_bits || clock::now() > deadline)
         break;
      }

   return bits_collected;
   }

size_t Entropy_Sources::poll_just(RandomNumberGenerator& rng, const std::string& the_src)
   {
   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      if(m_srcs[i]->name() == the_src)
         {
         return m_srcs[i]->poll(rng);
         }
      }

   return 0;
   }

Entropy_Sources::Entropy_Sources(const std::vector<std::string>& sources)
   {
   for(auto&& src_name : sources)
      {
      add_source(Entropy_Source::create(src_name));
      }
   }

Entropy_Sources& Entropy_Sources::global_sources()
   {
   static Entropy_Sources global_entropy_sources(BOTAN_ENTROPY_DEFAULT_SOURCES);

   return global_entropy_sources;
   }

}

/*
* Hash Functions
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_ADLER32)
#endif

#if defined(BOTAN_HAS_CRC24)
#endif

#if defined(BOTAN_HAS_CRC32)
#endif

#if defined(BOTAN_HAS_GOST_34_11)
#endif

#if defined(BOTAN_HAS_KECCAK)
#endif

#if defined(BOTAN_HAS_MD4)
#endif

#if defined(BOTAN_HAS_MD5)
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
#endif

#if defined(BOTAN_HAS_SHA1)
#endif

#if defined(BOTAN_HAS_SHA2_32)
#endif

#if defined(BOTAN_HAS_SHA2_64)
#endif

#if defined(BOTAN_HAS_SHA3)
#endif

#if defined(BOTAN_HAS_SHAKE)
#endif

#if defined(BOTAN_HAS_SKEIN_512)
#endif

#if defined(BOTAN_HAS_STREEBOG)
#endif

#if defined(BOTAN_HAS_SM3)
#endif

#if defined(BOTAN_HAS_TIGER)
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
#endif

#if defined(BOTAN_HAS_COMB4P)
#endif

#if defined(BOTAN_HAS_BLAKE2B)
#endif

#if defined(BOTAN_HAS_BEARSSL)
#endif

#if defined(BOTAN_HAS_OPENSSL)
#endif

#if defined(BOTAN_HAS_COMMONCRYPTO)
#endif

namespace Botan {

std::unique_ptr<HashFunction> HashFunction::create(const std::string& algo_spec,
                                                   const std::string& provider)
   {

#if defined(BOTAN_HAS_COMMONCRYPTO)
   if(provider.empty() || provider == "commoncrypto")
      {
      if(auto hash = make_commoncrypto_hash(algo_spec))
         return hash;

      if(!provider.empty())
         return nullptr;
      }
#endif

#if defined(BOTAN_HAS_OPENSSL)
   if(provider.empty() || provider == "openssl")
      {
      if(auto hash = make_openssl_hash(algo_spec))
         return hash;

      if(!provider.empty())
         return nullptr;
      }
#endif

#if defined(BOTAN_HAS_BEARSSL)
   if(provider.empty() || provider == "bearssl")
      {
      if(auto hash = make_bearssl_hash(algo_spec))
         return hash;

      if(!provider.empty())
         return nullptr;
      }
#endif

   if(provider.empty() == false && provider != "base")
      return nullptr; // unknown provider

#if defined(BOTAN_HAS_SHA1)
   if(algo_spec == "SHA-160" ||
      algo_spec == "SHA-1" ||
      algo_spec == "SHA1")
      {
      return std::unique_ptr<HashFunction>(new SHA_160);
      }
#endif

#if defined(BOTAN_HAS_SHA2_32)
   if(algo_spec == "SHA-224")
      {
      return std::unique_ptr<HashFunction>(new SHA_224);
      }

   if(algo_spec == "SHA-256")
      {
      return std::unique_ptr<HashFunction>(new SHA_256);
      }
#endif

#if defined(BOTAN_HAS_SHA2_64)
   if(algo_spec == "SHA-384")
      {
      return std::unique_ptr<HashFunction>(new SHA_384);
      }

   if(algo_spec == "SHA-512")
      {
      return std::unique_ptr<HashFunction>(new SHA_512);
      }

   if(algo_spec == "SHA-512-256")
      {
      return std::unique_ptr<HashFunction>(new SHA_512_256);
      }
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
   if(algo_spec == "RIPEMD-160")
      {
      return std::unique_ptr<HashFunction>(new RIPEMD_160);
      }
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
   if(algo_spec == "Whirlpool")
      {
      return std::unique_ptr<HashFunction>(new Whirlpool);
      }
#endif

#if defined(BOTAN_HAS_MD5)
   if(algo_spec == "MD5")
      {
      return std::unique_ptr<HashFunction>(new MD5);
      }
#endif

#if defined(BOTAN_HAS_MD4)
   if(algo_spec == "MD4")
      {
      return std::unique_ptr<HashFunction>(new MD4);
      }
#endif

#if defined(BOTAN_HAS_GOST_34_11)
   if(algo_spec == "GOST-R-34.11-94" || algo_spec == "GOST-34.11")
      {
      return std::unique_ptr<HashFunction>(new GOST_34_11);
      }
#endif

#if defined(BOTAN_HAS_ADLER32)
   if(algo_spec == "Adler32")
      {
      return std::unique_ptr<HashFunction>(new Adler32);
      }
#endif

#if defined(BOTAN_HAS_CRC24)
   if(algo_spec == "CRC24")
      {
      return std::unique_ptr<HashFunction>(new CRC24);
      }
#endif

#if defined(BOTAN_HAS_CRC32)
   if(algo_spec == "CRC32")
      {
      return std::unique_ptr<HashFunction>(new CRC32);
      }
#endif

   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_TIGER)
   if(req.algo_name() == "Tiger")
      {
      return std::unique_ptr<HashFunction>(
         new Tiger(req.arg_as_integer(0, 24),
                   req.arg_as_integer(1, 3)));
      }
#endif

#if defined(BOTAN_HAS_SKEIN_512)
   if(req.algo_name() == "Skein-512")
      {
      return std::unique_ptr<HashFunction>(
         new Skein_512(req.arg_as_integer(0, 512), req.arg(1, "")));
      }
#endif

#if defined(BOTAN_HAS_BLAKE2B)
   if(req.algo_name() == "Blake2b")
      {
      return std::unique_ptr<HashFunction>(
         new Blake2b(req.arg_as_integer(0, 512)));
   }
#endif

#if defined(BOTAN_HAS_KECCAK)
   if(req.algo_name() == "Keccak-1600")
      {
      return std::unique_ptr<HashFunction>(
         new Keccak_1600(req.arg_as_integer(0, 512)));
      }
#endif

#if defined(BOTAN_HAS_SHA3)
   if(req.algo_name() == "SHA-3")
      {
      return std::unique_ptr<HashFunction>(
         new SHA_3(req.arg_as_integer(0, 512)));
      }
#endif

#if defined(BOTAN_HAS_SHAKE)
   if(req.algo_name() == "SHAKE-128")
      {
      return std::unique_ptr<HashFunction>(new SHAKE_128(req.arg_as_integer(0, 128)));
      }
   if(req.algo_name() == "SHAKE-256")
      {
      return std::unique_ptr<HashFunction>(new SHAKE_256(req.arg_as_integer(0, 256)));
      }
#endif

#if defined(BOTAN_HAS_STREEBOG)
   if(algo_spec == "Streebog-256")
      {
      return std::unique_ptr<HashFunction>(new Streebog_256);
      }
   if(algo_spec == "Streebog-512")
      {
      return std::unique_ptr<HashFunction>(new Streebog_512);
      }
#endif

#if defined(BOTAN_HAS_SM3)
   if(algo_spec == "SM3")
      {
      return std::unique_ptr<HashFunction>(new SM3);
      }
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
   if(req.algo_name() == "Whirlpool")
      {
      return std::unique_ptr<HashFunction>(new Whirlpool);
      }
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
   if(req.algo_name() == "Parallel")
      {
      std::vector<std::unique_ptr<HashFunction>> hashes;

      for(size_t i = 0; i != req.arg_count(); ++i)
         {
         auto h = HashFunction::create(req.arg(i));
         if(!h)
            {
            return nullptr;
            }
         hashes.push_back(std::move(h));
         }

      return std::unique_ptr<HashFunction>(new Parallel(hashes));
      }
#endif

#if defined(BOTAN_HAS_COMB4P)
   if(req.algo_name() == "Comb4P" && req.arg_count() == 2)
      {
      std::unique_ptr<HashFunction> h1(HashFunction::create(req.arg(0)));
      std::unique_ptr<HashFunction> h2(HashFunction::create(req.arg(1)));

      if(h1 && h2)
         return std::unique_ptr<HashFunction>(new Comb4P(h1.release(), h2.release()));
      }
#endif


   return nullptr;
   }

//static
std::unique_ptr<HashFunction>
HashFunction::create_or_throw(const std::string& algo,
                              const std::string& provider)
   {
   if(auto hash = HashFunction::create(algo, provider))
      {
      return hash;
      }
   throw Lookup_Error("Hash", algo, provider);
   }

std::vector<std::string> HashFunction::providers(const std::string& algo_spec)
   {
   return probe_providers_of<HashFunction>(algo_spec, {"base", "bearssl", "openssl", "commoncrypto"});
   }

}

/*
* Hex Encoding and Decoding
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void hex_encode(char output[],
                const uint8_t input[],
                size_t input_length,
                bool uppercase)
   {
   static const uint8_t BIN_TO_HEX_UPPER[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'A', 'B', 'C', 'D', 'E', 'F' };

   static const uint8_t BIN_TO_HEX_LOWER[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f' };

   const uint8_t* tbl = uppercase ? BIN_TO_HEX_UPPER : BIN_TO_HEX_LOWER;

   for(size_t i = 0; i != input_length; ++i)
      {
      uint8_t x = input[i];
      output[2*i  ] = tbl[(x >> 4) & 0x0F];
      output[2*i+1] = tbl[(x     ) & 0x0F];
      }
   }

std::string hex_encode(const uint8_t input[],
                       size_t input_length,
                       bool uppercase)
   {
   std::string output(2 * input_length, 0);

   if(input_length)
      hex_encode(&output.front(), input, input_length, uppercase);

   return output;
   }

size_t hex_decode(uint8_t output[],
                  const char input[],
                  size_t input_length,
                  size_t& input_consumed,
                  bool ignore_ws)
   {
   /*
   * Mapping of hex characters to either their binary equivalent
   * or to an error code.
   *  If valid hex (0-9 A-F a-f), the value.
   *  If whitespace, then 0x80
   *  Otherwise 0xFF
   * Warning: this table assumes ASCII character encodings
   */

   static const uint8_t HEX_TO_BIN[256] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80,
      0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01,
      0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
      0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C,
      0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

   uint8_t* out_ptr = output;
   bool top_nibble = true;

   clear_mem(output, input_length / 2);

   for(size_t i = 0; i != input_length; ++i)
      {
      const uint8_t bin = HEX_TO_BIN[static_cast<uint8_t>(input[i])];

      if(bin >= 0x10)
         {
         if(bin == 0x80 && ignore_ws)
            continue;

         std::string bad_char(1, input[i]);
         if(bad_char == "\t")
           bad_char = "\\t";
         else if(bad_char == "\n")
           bad_char = "\\n";

         throw Invalid_Argument(
           std::string("hex_decode: invalid hex character '") +
           bad_char + "'");
         }

      if(top_nibble)
         *out_ptr |= bin << 4;
      else
         *out_ptr |= bin;

      top_nibble = !top_nibble;
      if(top_nibble)
         ++out_ptr;
      }

   input_consumed = input_length;
   size_t written = (out_ptr - output);

   /*
   * We only got half of a uint8_t at the end; zap the half-written
   * output and mark it as unread
   */
   if(!top_nibble)
      {
      *out_ptr = 0;
      input_consumed -= 1;
      }

   return written;
   }

size_t hex_decode(uint8_t output[],
                  const char input[],
                  size_t input_length,
                  bool ignore_ws)
   {
   size_t consumed = 0;
   size_t written = hex_decode(output, input, input_length,
                               consumed, ignore_ws);

   if(consumed != input_length)
      throw Invalid_Argument("hex_decode: input did not have full bytes");

   return written;
   }

size_t hex_decode(uint8_t output[],
                  const std::string& input,
                  bool ignore_ws)
   {
   return hex_decode(output, input.data(), input.length(), ignore_ws);
   }

secure_vector<uint8_t> hex_decode_locked(const char input[],
                                      size_t input_length,
                                      bool ignore_ws)
   {
   secure_vector<uint8_t> bin(1 + input_length / 2);

   size_t written = hex_decode(bin.data(),
                               input,
                               input_length,
                               ignore_ws);

   bin.resize(written);
   return bin;
   }

secure_vector<uint8_t> hex_decode_locked(const std::string& input,
                                      bool ignore_ws)
   {
   return hex_decode_locked(input.data(), input.size(), ignore_ws);
   }

std::vector<uint8_t> hex_decode(const char input[],
                             size_t input_length,
                             bool ignore_ws)
   {
   std::vector<uint8_t> bin(1 + input_length / 2);

   size_t written = hex_decode(bin.data(),
                               input,
                               input_length,
                               ignore_ws);

   bin.resize(written);
   return bin;
   }

std::vector<uint8_t> hex_decode(const std::string& input,
                             bool ignore_ws)
   {
   return hex_decode(input.data(), input.size(), ignore_ws);
   }

}
/*
* Message Authentication Code base class
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_CBC_MAC)
#endif

#if defined(BOTAN_HAS_CMAC)
#endif

#if defined(BOTAN_HAS_GMAC)
#endif

#if defined(BOTAN_HAS_HMAC)
#endif

#if defined(BOTAN_HAS_POLY1305)
#endif

#if defined(BOTAN_HAS_SIPHASH)
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
#endif

namespace Botan {

std::unique_ptr<MessageAuthenticationCode>
MessageAuthenticationCode::create(const std::string& algo_spec,
                                  const std::string& provider)
   {
   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_GMAC)
   if(req.algo_name() == "GMAC" && req.arg_count() == 1)
      {
      if(provider.empty() || provider == "base")
         {
         if(auto bc = BlockCipher::create(req.arg(0)))
            return std::unique_ptr<MessageAuthenticationCode>(new GMAC(bc.release()));
         }
      }
#endif

#if defined(BOTAN_HAS_HMAC)
   if(req.algo_name() == "HMAC" && req.arg_count() == 1)
      {
      // TODO OpenSSL
      if(provider.empty() || provider == "base")
         {
         if(auto h = HashFunction::create(req.arg(0)))
            return std::unique_ptr<MessageAuthenticationCode>(new HMAC(h.release()));
         }
      }
#endif

#if defined(BOTAN_HAS_POLY1305)
   if(req.algo_name() == "Poly1305" && req.arg_count() == 0)
      {
      if(provider.empty() || provider == "base")
         return std::unique_ptr<MessageAuthenticationCode>(new Poly1305);
      }
#endif

#if defined(BOTAN_HAS_SIPHASH)
   if(req.algo_name() == "SipHash")
      {
      if(provider.empty() || provider == "base")
         {
         return std::unique_ptr<MessageAuthenticationCode>(
            new SipHash(req.arg_as_integer(0, 2), req.arg_as_integer(1, 4)));
         }
      }
#endif

#if defined(BOTAN_HAS_CMAC)
   if((req.algo_name() == "CMAC" || req.algo_name() == "OMAC") && req.arg_count() == 1)
      {
      // TODO: OpenSSL CMAC
      if(provider.empty() || provider == "base")
         {
         if(auto bc = BlockCipher::create(req.arg(0)))
            return std::unique_ptr<MessageAuthenticationCode>(new CMAC(bc.release()));
         }
      }
#endif


#if defined(BOTAN_HAS_CBC_MAC)
   if(req.algo_name() == "CBC-MAC" && req.arg_count() == 1)
      {
      if(provider.empty() || provider == "base")
         {
         if(auto bc = BlockCipher::create(req.arg(0)))
            return std::unique_ptr<MessageAuthenticationCode>(new CBC_MAC(bc.release()));
         }
      }
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
   if(req.algo_name() == "X9.19-MAC")
      {
      if(provider.empty() || provider == "base")
         {
         return std::unique_ptr<MessageAuthenticationCode>(new ANSI_X919_MAC);
         }
      }
#endif

   BOTAN_UNUSED(req);
   BOTAN_UNUSED(provider);

   return nullptr;
   }

std::vector<std::string>
MessageAuthenticationCode::providers(const std::string& algo_spec)
   {
   return probe_providers_of<MessageAuthenticationCode>(algo_spec, {"base", "openssl"});
   }

//static
std::unique_ptr<MessageAuthenticationCode>
MessageAuthenticationCode::create_or_throw(const std::string& algo,
                                           const std::string& provider)
   {
   if(auto mac = MessageAuthenticationCode::create(algo, provider))
      {
      return mac;
      }
   throw Lookup_Error("MAC", algo, provider);
   }

void MessageAuthenticationCode::start_msg(const uint8_t nonce[], size_t nonce_len)
   {
   BOTAN_UNUSED(nonce);
   if(nonce_len > 0)
      throw Invalid_IV_Length(name(), nonce_len);
   }

/*
* Default (deterministic) MAC verification operation
*/
bool MessageAuthenticationCode::verify_mac(const uint8_t mac[], size_t length)
   {
   secure_vector<uint8_t> our_mac = final();

   if(our_mac.size() != length)
      return false;

   return constant_time_compare(our_mac.data(), mac, length);
   }

}
/*
* Cipher Modes
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_BLOCK_CIPHER)
#endif

#if defined(BOTAN_HAS_AEAD_MODES)
#endif

#if defined(BOTAN_HAS_MODE_CBC)
#endif

#if defined(BOTAN_HAS_MODE_CFB)
#endif

#if defined(BOTAN_HAS_MODE_XTS)
#endif

#if defined(BOTAN_HAS_OPENSSL)
#endif

#if defined(BOTAN_HAS_COMMONCRYPTO)
#endif

namespace Botan {

std::unique_ptr<Cipher_Mode> Cipher_Mode::create_or_throw(const std::string& algo,
                                                          Cipher_Dir direction,
                                                          const std::string& provider)
   {
   if(auto mode = Cipher_Mode::create(algo, direction, provider))
      return mode;

   throw Lookup_Error("Cipher mode", algo, provider);
   }

std::unique_ptr<Cipher_Mode> Cipher_Mode::create(const std::string& algo,
                                                 Cipher_Dir direction,
                                                 const std::string& provider)
   {
#if defined(BOTAN_HAS_COMMONCRYPTO)
   if(provider.empty() || provider == "commoncrypto")
      {
      std::unique_ptr<Cipher_Mode> commoncrypto_cipher(make_commoncrypto_cipher_mode(algo, direction));

      if(commoncrypto_cipher)
         return commoncrypto_cipher;

      if(!provider.empty())
         return std::unique_ptr<Cipher_Mode>();
      }
#endif

#if defined(BOTAN_HAS_OPENSSL)
   if(provider.empty() || provider == "openssl")
      {
      std::unique_ptr<Cipher_Mode> openssl_cipher(make_openssl_cipher_mode(algo, direction));

      if(openssl_cipher)
         return openssl_cipher;

      if(!provider.empty())
         return std::unique_ptr<Cipher_Mode>();
      }
#endif

#if defined(BOTAN_HAS_STREAM_CIPHER)
   if(auto sc = StreamCipher::create(algo))
      {
      return std::unique_ptr<Cipher_Mode>(new Stream_Cipher_Mode(sc.release()));
      }
#endif

#if defined(BOTAN_HAS_AEAD_MODES)
   if(auto aead = AEAD_Mode::create(algo, direction))
      {
      return std::unique_ptr<Cipher_Mode>(aead.release());
      }
#endif

   if(algo.find('/') != std::string::npos)
      {
      const std::vector<std::string> algo_parts = split_on(algo, '/');
      const std::string cipher_name = algo_parts[0];
      const std::vector<std::string> mode_info = parse_algorithm_name(algo_parts[1]);

      if(mode_info.empty())
         return std::unique_ptr<Cipher_Mode>();

      std::ostringstream alg_args;

      alg_args << '(' << cipher_name;
      for(size_t i = 1; i < mode_info.size(); ++i)
         alg_args << ',' << mode_info[i];
      for(size_t i = 2; i < algo_parts.size(); ++i)
         alg_args << ',' << algo_parts[i];
      alg_args << ')';

      const std::string mode_name = mode_info[0] + alg_args.str();
      return Cipher_Mode::create(mode_name, direction, provider);
      }

#if defined(BOTAN_HAS_BLOCK_CIPHER)

   SCAN_Name spec(algo);

   if(spec.arg_count() == 0)
      {
      return std::unique_ptr<Cipher_Mode>();
      }

   std::unique_ptr<BlockCipher> bc(BlockCipher::create(spec.arg(0), provider));

   if(!bc)
      {
      return std::unique_ptr<Cipher_Mode>();
      }

#if defined(BOTAN_HAS_MODE_CBC)
   if(spec.algo_name() == "CBC")
      {
      const std::string padding = spec.arg(1, "PKCS7");

      if(padding == "CTS")
         {
         if(direction == ENCRYPTION)
            return std::unique_ptr<Cipher_Mode>(new CTS_Encryption(bc.release()));
         else
            return std::unique_ptr<Cipher_Mode>(new CTS_Decryption(bc.release()));
         }
      else
         {
         std::unique_ptr<BlockCipherModePaddingMethod> pad(get_bc_pad(padding));

         if(pad)
            {
            if(direction == ENCRYPTION)
               return std::unique_ptr<Cipher_Mode>(new CBC_Encryption(bc.release(), pad.release()));
            else
               return std::unique_ptr<Cipher_Mode>(new CBC_Decryption(bc.release(), pad.release()));
            }
         }
      }
#endif

#if defined(BOTAN_HAS_MODE_XTS)
   if(spec.algo_name() == "XTS")
      {
      if(direction == ENCRYPTION)
         return std::unique_ptr<Cipher_Mode>(new XTS_Encryption(bc.release()));
      else
         return std::unique_ptr<Cipher_Mode>(new XTS_Decryption(bc.release()));
      }
#endif

#if defined(BOTAN_HAS_MODE_CFB)
   if(spec.algo_name() == "CFB")
      {
      const size_t feedback_bits = spec.arg_as_integer(1, 8*bc->block_size());
      if(direction == ENCRYPTION)
         return std::unique_ptr<Cipher_Mode>(new CFB_Encryption(bc.release(), feedback_bits));
      else
         return std::unique_ptr<Cipher_Mode>(new CFB_Decryption(bc.release(), feedback_bits));
      }
#endif

#endif

   return std::unique_ptr<Cipher_Mode>();
   }

//static
std::vector<std::string> Cipher_Mode::providers(const std::string& algo_spec)
   {
   const std::vector<std::string>& possible = { "base", "openssl", "commoncrypto" };
   std::vector<std::string> providers;
   for(auto&& prov : possible)
      {
      std::unique_ptr<Cipher_Mode> mode = Cipher_Mode::create(algo_spec, ENCRYPTION, prov);
      if(mode)
         {
         providers.push_back(prov); // available
         }
      }
   return providers;
   }

}
/*
* Derived from poly1305-donna-64.h by Andrew Moon <liquidsun@gmail.com>
* in https://github.com/floodyberry/poly1305-donna
*
* (C) 2014 Andrew Moon
* (C) 2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

void poly1305_init(secure_vector<uint64_t>& X, const uint8_t key[32])
   {
   /* r &= 0xffffffc0ffffffc0ffffffc0fffffff */
   const uint64_t t0 = load_le<uint64_t>(key, 0);
   const uint64_t t1 = load_le<uint64_t>(key, 1);

   X[0] = ( t0                    ) & 0xffc0fffffff;
   X[1] = ((t0 >> 44) | (t1 << 20)) & 0xfffffc0ffff;
   X[2] = ((t1 >> 24)             ) & 0x00ffffffc0f;

   /* h = 0 */
   X[3] = 0;
   X[4] = 0;
   X[5] = 0;

   /* save pad for later */
   X[6] = load_le<uint64_t>(key, 2);
   X[7] = load_le<uint64_t>(key, 3);
   }

void poly1305_blocks(secure_vector<uint64_t>& X, const uint8_t *m, size_t blocks, bool is_final = false)
   {
#if !defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
   typedef donna128 uint128_t;
#endif

   const uint64_t hibit = is_final ? 0 : (static_cast<uint64_t>(1) << 40); /* 1 << 128 */

   const uint64_t r0 = X[0];
   const uint64_t r1 = X[1];
   const uint64_t r2 = X[2];

   const uint64_t M44 = 0xFFFFFFFFFFF;
   const uint64_t M42 = 0x3FFFFFFFFFF;

   uint64_t h0 = X[3+0];
   uint64_t h1 = X[3+1];
   uint64_t h2 = X[3+2];

   const uint64_t s1 = r1 * 20;
   const uint64_t s2 = r2 * 20;

   for(size_t i = 0; i != blocks; ++i)
      {
      const uint64_t t0 = load_le<uint64_t>(m, 0);
      const uint64_t t1 = load_le<uint64_t>(m, 1);

      h0 += (( t0                    ) & M44);
      h1 += (((t0 >> 44) | (t1 << 20)) & M44);
      h2 += (((t1 >> 24)             ) & M42) | hibit;

      const uint128_t d0 = uint128_t(h0) * r0 + uint128_t(h1) * s2 + uint128_t(h2) * s1;
      const uint64_t c0 = carry_shift(d0, 44);

      const uint128_t d1 = uint128_t(h0) * r1 + uint128_t(h1) * r0 + uint128_t(h2) * s2 + c0;
      const uint64_t c1 = carry_shift(d1, 44);

      const uint128_t d2 = uint128_t(h0) * r2 + uint128_t(h1) * r1 + uint128_t(h2) * r0 + c1;
      const uint64_t c2 = carry_shift(d2, 42);

      h0 = d0 & M44;
      h1 = d1 & M44;
      h2 = d2 & M42;

      h0 += c2 * 5;
      h1 += carry_shift(h0, 44);
      h0 = h0 & M44;

      m += 16;
      }

   X[3+0] = h0;
   X[3+1] = h1;
   X[3+2] = h2;
   }

void poly1305_finish(secure_vector<uint64_t>& X, uint8_t mac[16])
   {
   const uint64_t M44 = 0xFFFFFFFFFFF;
   const uint64_t M42 = 0x3FFFFFFFFFF;

   /* fully carry h */
   uint64_t h0 = X[3+0];
   uint64_t h1 = X[3+1];
   uint64_t h2 = X[3+2];

   uint64_t c;
                c = (h1 >> 44); h1 &= M44;
   h2 += c;     c = (h2 >> 42); h2 &= M42;
   h0 += c * 5; c = (h0 >> 44); h0 &= M44;
   h1 += c;     c = (h1 >> 44); h1 &= M44;
   h2 += c;     c = (h2 >> 42); h2 &= M42;
   h0 += c * 5; c = (h0 >> 44); h0 &= M44;
   h1 += c;

   /* compute h + -p */
   uint64_t g0 = h0 + 5; c = (g0 >> 44); g0 &= M44;
   uint64_t g1 = h1 + c; c = (g1 >> 44); g1 &= M44;
   uint64_t g2 = h2 + c - (static_cast<uint64_t>(1) << 42);

   /* select h if h < p, or h + -p if h >= p */
   const auto c_mask = CT::Mask<uint64_t>::expand(c);
   h0 = c_mask.select(g0, h0);
   h1 = c_mask.select(g1, h1);
   h2 = c_mask.select(g2, h2);

   /* h = (h + pad) */
   const uint64_t t0 = X[6];
   const uint64_t t1 = X[7];

   h0 += (( t0                    ) & M44)    ; c = (h0 >> 44); h0 &= M44;
   h1 += (((t0 >> 44) | (t1 << 20)) & M44) + c; c = (h1 >> 44); h1 &= M44;
   h2 += (((t1 >> 24)             ) & M42) + c;                 h2 &= M42;

   /* mac = h % (2^128) */
   h0 = ((h0      ) | (h1 << 44));
   h1 = ((h1 >> 20) | (h2 << 24));

   store_le(mac, h0, h1);

   /* zero out the state */
   clear_mem(X.data(), X.size());
   }

}

void Poly1305::clear()
   {
   zap(m_poly);
   zap(m_buf);
   m_buf_pos = 0;
   }

void Poly1305::key_schedule(const uint8_t key[], size_t)
   {
   m_buf_pos = 0;
   m_buf.resize(16);
   m_poly.resize(8);

   poly1305_init(m_poly, key);
   }

void Poly1305::add_data(const uint8_t input[], size_t length)
   {
   verify_key_set(m_poly.size() == 8);

   if(m_buf_pos)
      {
      buffer_insert(m_buf, m_buf_pos, input, length);

      if(m_buf_pos + length >= m_buf.size())
         {
         poly1305_blocks(m_poly, m_buf.data(), 1);
         input += (m_buf.size() - m_buf_pos);
         length -= (m_buf.size() - m_buf_pos);
         m_buf_pos = 0;
         }
      }

   const size_t full_blocks = length / m_buf.size();
   const size_t remaining   = length % m_buf.size();

   if(full_blocks)
      poly1305_blocks(m_poly, input, full_blocks);

   buffer_insert(m_buf, m_buf_pos, input + full_blocks * m_buf.size(), remaining);
   m_buf_pos += remaining;
   }

void Poly1305::final_result(uint8_t out[])
   {
   verify_key_set(m_poly.size() == 8);

   if(m_buf_pos != 0)
      {
      m_buf[m_buf_pos] = 1;
      const size_t len = m_buf.size() - m_buf_pos - 1;
      if (len > 0)
         {
         clear_mem(&m_buf[m_buf_pos+1], len);
         }
      poly1305_blocks(m_poly, m_buf.data(), 1, true);
      }

   poly1305_finish(m_poly, out);

   m_poly.clear();
   m_buf_pos = 0;
   }

}
/*
* Entropy Source Using Intel's rdrand instruction
* (C) 2012,2015 Jack Lloyd
* (C) 2015 Daniel Neus
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

size_t Intel_Rdrand::poll(RandomNumberGenerator& rng)
   {
   if(BOTAN_ENTROPY_INTEL_RNG_POLLS > 0 && RDRAND_RNG::available())
      {
      RDRAND_RNG rdrand_rng;
      secure_vector<uint8_t> buf(4 * BOTAN_ENTROPY_INTEL_RNG_POLLS);

      rdrand_rng.randomize(buf.data(), buf.size());
      rng.add_entropy(buf.data(), buf.size());
      }

   // RDRAND is used but not trusted
   return 0;
   }

}
/*
* RDRAND RNG
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if !defined(BOTAN_USE_GCC_INLINE_ASM)
  #include <immintrin.h>
#endif

namespace Botan {

RDRAND_RNG::RDRAND_RNG()
   {
   if(!RDRAND_RNG::available())
      throw Invalid_State("Current CPU does not support RDRAND instruction");
   }

//static
bool RDRAND_RNG::available()
   {
   return CPUID::has_rdrand();
   }

//static
uint32_t RDRAND_RNG::rdrand()
   {
   for(;;)
      {
      bool ok = false;
      uint32_t r = rdrand_status(ok);
      if(ok)
         return r;
      }
   }

//static
BOTAN_FUNC_ISA("rdrnd")
uint32_t RDRAND_RNG::rdrand_status(bool& ok)
   {
   ok = false;
   uint32_t r = 0;

   for(size_t i = 0; i != BOTAN_ENTROPY_RDRAND_RETRIES; ++i)
      {
#if defined(BOTAN_USE_GCC_INLINE_ASM)
      int cf = 0;

      // Encoding of rdrand %eax
      asm(".byte 0x0F, 0xC7, 0xF0; adcl $0,%1" :
          "=a" (r), "=r" (cf) : "0" (r), "1" (cf) : "cc");
#else
      int cf = _rdrand32_step(&r);
#endif
      if(1 == cf)
         {
         ok = true;
         break;
         }
      }

   return r;
   }

void RDRAND_RNG::randomize(uint8_t out[], size_t out_len)
   {
   while(out_len >= 4)
      {
      uint32_t r = RDRAND_RNG::rdrand();

      store_le(r, out);
      out += 4;
      out_len -= 4;
      }

   if(out_len) // between 1 and 3 trailing bytes
      {
      uint32_t r = RDRAND_RNG::rdrand();
      for(size_t i = 0; i != out_len; ++i)
         out[i] = get_byte(i, r);
      }
   }

}
/*
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
#endif

namespace Botan {

void RandomNumberGenerator::randomize_with_ts_input(uint8_t output[], size_t output_len)
   {
   if(this->accepts_input())
      {
      /*
      Form additional input which is provided to the PRNG implementation
      to paramaterize the KDF output.
      */
      uint8_t additional_input[16] = { 0 };
      store_le(OS::get_system_timestamp_ns(), additional_input);
      store_le(OS::get_high_resolution_clock(), additional_input + 8);

      this->randomize_with_input(output, output_len, additional_input, sizeof(additional_input));
      }
   else
      {
      this->randomize(output, output_len);
      }
   }

void RandomNumberGenerator::randomize_with_input(uint8_t output[], size_t output_len,
                                                 const uint8_t input[], size_t input_len)
   {
   this->add_entropy(input, input_len);
   this->randomize(output, output_len);
   }

size_t RandomNumberGenerator::reseed(Entropy_Sources& srcs,
                                     size_t poll_bits,
                                     std::chrono::milliseconds poll_timeout)
   {
   if(this->accepts_input())
      {
      return srcs.poll(*this, poll_bits, poll_timeout);
      }
   else
      {
      return 0;
      }
   }

void RandomNumberGenerator::reseed_from_rng(RandomNumberGenerator& rng, size_t poll_bits)
   {
   if(this->accepts_input())
      {
      secure_vector<uint8_t> buf(poll_bits / 8);
      rng.randomize(buf.data(), buf.size());
      this->add_entropy(buf.data(), buf.size());
      }
   }

RandomNumberGenerator* RandomNumberGenerator::make_rng()
   {
#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
   return new AutoSeeded_RNG;
#else
   throw Not_Implemented("make_rng failed, no AutoSeeded_RNG in this build");
#endif
   }

#if defined(BOTAN_TARGET_OS_HAS_THREADS)

#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
Serialized_RNG::Serialized_RNG() : m_rng(new AutoSeeded_RNG) {}
#else
Serialized_RNG::Serialized_RNG()
   {
   throw Not_Implemented("Serialized_RNG default constructor failed: AutoSeeded_RNG disabled in build");
   }
#endif

#endif

}
/*
* Stream Ciphers
* (C) 2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_CHACHA)
#endif

#if defined(BOTAN_HAS_SALSA20)
#endif

#if defined(BOTAN_HAS_SHAKE_CIPHER)
#endif

#if defined(BOTAN_HAS_CTR_BE)
#endif

#if defined(BOTAN_HAS_OFB)
#endif

#if defined(BOTAN_HAS_RC4)
#endif

#if defined(BOTAN_HAS_OPENSSL)
#endif

namespace Botan {

std::unique_ptr<StreamCipher> StreamCipher::create(const std::string& algo_spec,
                                                   const std::string& provider)
   {
   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_CTR_BE)
   if((req.algo_name() == "CTR-BE" || req.algo_name() == "CTR") && req.arg_count_between(1,2))
      {
      if(provider.empty() || provider == "base")
         {
         auto cipher = BlockCipher::create(req.arg(0));
         if(cipher)
            {
            size_t ctr_size = req.arg_as_integer(1, cipher->block_size());
            return std::unique_ptr<StreamCipher>(new CTR_BE(cipher.release(), ctr_size));
            }
         }
      }
#endif

#if defined(BOTAN_HAS_CHACHA)
   if(req.algo_name() == "ChaCha")
      {
      if(provider.empty() || provider == "base")
         return std::unique_ptr<StreamCipher>(new ChaCha(req.arg_as_integer(0, 20)));
      }

   if(req.algo_name() == "ChaCha20")
      {
      if(provider.empty() || provider == "base")
         return std::unique_ptr<StreamCipher>(new ChaCha(20));
      }
#endif

#if defined(BOTAN_HAS_SALSA20)
   if(req.algo_name() == "Salsa20")
      {
      if(provider.empty() || provider == "base")
         return std::unique_ptr<StreamCipher>(new Salsa20);
      }
#endif

#if defined(BOTAN_HAS_SHAKE_CIPHER)
   if(req.algo_name() == "SHAKE-128")
      {
      if(provider.empty() || provider == "base")
         return std::unique_ptr<StreamCipher>(new SHAKE_128_Cipher);
      }
#endif

#if defined(BOTAN_HAS_OFB)
   if(req.algo_name() == "OFB" && req.arg_count() == 1)
      {
      if(provider.empty() || provider == "base")
         {
         if(auto c = BlockCipher::create(req.arg(0)))
            return std::unique_ptr<StreamCipher>(new OFB(c.release()));
         }
      }
#endif

#if defined(BOTAN_HAS_RC4)

   if(req.algo_name() == "RC4" ||
      req.algo_name() == "ARC4" ||
      req.algo_name() == "MARK-4")
      {
      const size_t skip = (req.algo_name() == "MARK-4") ? 256 : req.arg_as_integer(0, 0);

#if defined(BOTAN_HAS_OPENSSL)
      if(provider.empty() || provider == "openssl")
         {
         return std::unique_ptr<StreamCipher>(make_openssl_rc4(skip));
         }
#endif

      if(provider.empty() || provider == "base")
         {
         return std::unique_ptr<StreamCipher>(new RC4(skip));
         }
      }

#endif

   BOTAN_UNUSED(req);
   BOTAN_UNUSED(provider);

   return nullptr;
   }

//static
std::unique_ptr<StreamCipher>
StreamCipher::create_or_throw(const std::string& algo,
                             const std::string& provider)
   {
   if(auto sc = StreamCipher::create(algo, provider))
      {
      return sc;
      }
   throw Lookup_Error("Stream cipher", algo, provider);
   }

std::vector<std::string> StreamCipher::providers(const std::string& algo_spec)
   {
   return probe_providers_of<StreamCipher>(algo_spec, {"base", "openssl"});
   }

}
/*
* System RNG
* (C) 2014,2015,2017,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_RTLGENRANDOM)
  #define NOMINMAX 1
  #define _WINSOCKAPI_ // stop windows.h including winsock.h
  #include <windows.h>

#elif defined(BOTAN_TARGET_OS_HAS_CRYPTO_NG)
   #include <bcrypt.h>

#elif defined(BOTAN_TARGET_OS_HAS_ARC4RANDOM)
   #include <stdlib.h>

#elif defined(BOTAN_TARGET_OS_HAS_GETRANDOM)
   #include <sys/random.h>
   #include <errno.h>

#elif defined(BOTAN_TARGET_OS_HAS_DEV_RANDOM)
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
   #include <unistd.h>
   #include <errno.h>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_OS_HAS_RTLGENRANDOM)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      System_RNG_Impl() : m_advapi("advapi32.dll")
         {
         // This throws if the function is not found
         m_rtlgenrandom = m_advapi.resolve<RtlGenRandom_fptr>("SystemFunction036");
         }

      void randomize(uint8_t buf[], size_t len) override
         {
         bool success = m_rtlgenrandom(buf, ULONG(len)) == TRUE;
         if(!success)
            throw System_Error("RtlGenRandom failed");
         }

      void add_entropy(const uint8_t[], size_t) override { /* ignored */ }
      bool is_seeded() const override { return true; }
      bool accepts_input() const override { return false; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "RtlGenRandom"; }
   private:
      // Use type BYTE instead of BOOLEAN because of a naming conflict
      // https://msdn.microsoft.com/en-us/library/windows/desktop/aa387694(v=vs.85).aspx
      // https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
      using RtlGenRandom_fptr = BYTE (NTAPI *)(PVOID, ULONG);

      Dynamically_Loaded_Library m_advapi;
      RtlGenRandom_fptr m_rtlgenrandom;
   };

#elif defined(BOTAN_TARGET_OS_HAS_CRYPTO_NG)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      System_RNG_Impl()
         {
         NTSTATUS ret = ::BCryptOpenAlgorithmProvider(&m_prov,
                                                      BCRYPT_RNG_ALGORITHM,
                                                      MS_PRIMITIVE_PROVIDER, 0);
         if(ret != STATUS_SUCCESS)
            throw System_Error("System_RNG failed to acquire crypto provider", ret);
         }

      ~System_RNG_Impl()
         {
         ::BCryptCloseAlgorithmProvider(m_prov, 0);
         }

      void randomize(uint8_t buf[], size_t len) override
         {
         NTSTATUS ret = ::BCryptGenRandom(m_prov, static_cast<PUCHAR>(buf), static_cast<ULONG>(len), 0);
         if(ret != STATUS_SUCCESS)
            throw System_Error("System_RNG call to BCryptGenRandom failed", ret);
         }

      void add_entropy(const uint8_t in[], size_t length) override
         {
         /*
         There is a flag BCRYPT_RNG_USE_ENTROPY_IN_BUFFER to provide
         entropy inputs, but it is ignored in Windows 8 and later.
         */
         }

      bool is_seeded() const override { return true; }
      bool accepts_input() const override { return false; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "crypto_ng"; }
   private:
      BCRYPT_ALG_HANDLE m_handle;
   };

#elif defined(BOTAN_TARGET_OS_HAS_ARC4RANDOM)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      // No constructor or destructor needed as no userland state maintained

      void randomize(uint8_t buf[], size_t len) override
         {
         ::arc4random_buf(buf, len);
         }

      bool accepts_input() const override { return false; }
      void add_entropy(const uint8_t[], size_t) override { /* ignored */ }
      bool is_seeded() const override { return true; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "arc4random"; }
   };

#elif defined(BOTAN_TARGET_OS_HAS_GETRANDOM)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      // No constructor or destructor needed as no userland state maintained

      void randomize(uint8_t buf[], size_t len) override
         {
         const unsigned int flags = 0;

         while(len > 0)
            {
            const ssize_t got = ::getrandom(buf, len, flags);

            if(got < 0)
               {
               if(errno == EINTR)
                  continue;
               throw System_Error("System_RNG getrandom failed", errno);
               }

            buf += got;
            len -= got;
            }
         }

      bool accepts_input() const override { return false; }
      void add_entropy(const uint8_t[], size_t) override { /* ignored */ }
      bool is_seeded() const override { return true; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "getrandom"; }
   };


#elif defined(BOTAN_TARGET_OS_HAS_DEV_RANDOM)

// Read a random device

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      System_RNG_Impl()
         {
         #ifndef O_NOCTTY
            #define O_NOCTTY 0
         #endif

         m_fd = ::open(BOTAN_SYSTEM_RNG_DEVICE, O_RDWR | O_NOCTTY);

         if(m_fd >= 0)
            {
            m_writable = true;
            }
         else
            {
            /*
            Cannot open in read-write mode. Fall back to read-only,
            calls to add_entropy will fail, but randomize will work
            */
            m_fd = ::open(BOTAN_SYSTEM_RNG_DEVICE, O_RDONLY | O_NOCTTY);
            m_writable = false;
            }

         if(m_fd < 0)
            throw System_Error("System_RNG failed to open RNG device", errno);
         }

      ~System_RNG_Impl()
         {
         ::close(m_fd);
         m_fd = -1;
         }

      void randomize(uint8_t buf[], size_t len) override;
      void add_entropy(const uint8_t in[], size_t length) override;
      bool is_seeded() const override { return true; }
      bool accepts_input() const override { return m_writable; }
      void clear() override { /* not possible */ }
      std::string name() const override { return BOTAN_SYSTEM_RNG_DEVICE; }
   private:
      int m_fd;
      bool m_writable;
   };

void System_RNG_Impl::randomize(uint8_t buf[], size_t len)
   {
   while(len)
      {
      ssize_t got = ::read(m_fd, buf, len);

      if(got < 0)
         {
         if(errno == EINTR)
            continue;
         throw System_Error("System_RNG read failed", errno);
         }
      if(got == 0)
         throw System_Error("System_RNG EOF on device"); // ?!?

      buf += got;
      len -= got;
      }
   }

void System_RNG_Impl::add_entropy(const uint8_t input[], size_t len)
   {
   if(!m_writable)
      return;

   while(len)
      {
      ssize_t got = ::write(m_fd, input, len);

      if(got < 0)
         {
         if(errno == EINTR)
            continue;

         /*
         * This is seen on OS X CI, despite the fact that the man page
         * for Darwin urandom explicitly states that writing to it is
         * supported, and write(2) does not document EPERM at all.
         * But in any case EPERM seems indicative of a policy decision
         * by the OS or sysadmin that additional entropy is not wanted
         * in the system pool, so we accept that and return here,
         * since there is no corrective action possible.
	 *
	 * In Linux EBADF or EPERM is returned if m_fd is not opened for
	 * writing.
         */
         if(errno == EPERM || errno == EBADF)
            return;

         // maybe just ignore any failure here and return?
         throw System_Error("System_RNG write failed", errno);
         }

      input += got;
      len -= got;
      }
   }

#endif

}

RandomNumberGenerator& system_rng()
   {
   static System_RNG_Impl g_system_rng;
   return g_system_rng;
   }

}
/*
* Runtime assertion checking
* (C) 2010,2012,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void throw_invalid_argument(const char* message,
                            const char* func,
                            const char* file)
   {
   std::ostringstream format;
   format << message << " in " << func << ":" << file;
   throw Invalid_Argument(format.str());
   }

void throw_invalid_state(const char* expr,
                         const char* func,
                         const char* file)
   {
   std::ostringstream format;
   format << "Invalid state: " << expr << " was false in " << func << ":" << file;
   throw Invalid_State(format.str());
   }

void assertion_failure(const char* expr_str,
                       const char* assertion_made,
                       const char* func,
                       const char* file,
                       int line)
   {
   std::ostringstream format;

   format << "False assertion ";

   if(assertion_made && assertion_made[0] != 0)
      format << "'" << assertion_made << "' (expression " << expr_str << ") ";
   else
      format << expr_str << " ";

   if(func)
      format << "in " << func << " ";

   format << "@" << file << ":" << line;

   throw Internal_Error(format.str());
   }

}
/*
* Calendar Functions
* (C) 1999-2010,2017 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <ctime>
#include <iomanip>
#include <stdlib.h>

namespace Botan {

namespace {

std::tm do_gmtime(std::time_t time_val)
   {
   std::tm tm;

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   ::gmtime_s(&tm, &time_val); // Windows
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   ::gmtime_r(&time_val, &tm); // Unix/SUSv2
#else
   std::tm* tm_p = std::gmtime(&time_val);
   if (tm_p == nullptr)
      throw Encoding_Error("time_t_to_tm could not convert");
   tm = *tm_p;
#endif

   return tm;
   }

/*
Portable replacement for timegm, _mkgmtime, etc

Algorithm due to Howard Hinnant

See https://howardhinnant.github.io/date_algorithms.html#days_from_civil
for details and explaination. The code is slightly simplified by our assumption
that the date is at least 1970, which is sufficient for our purposes.
*/
size_t days_since_epoch(uint32_t year, uint32_t month, uint32_t day)
   {
   if(month <= 2)
      year -= 1;
   const uint32_t era = year / 400;
   const uint32_t yoe = year - era * 400;      // [0, 399]
   const uint32_t doy = (153*(month + (month > 2 ? -3 : 9)) + 2)/5 + day-1;  // [0, 365]
   const uint32_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
   return era * 146097 + doe - 719468;
   }

}

std::chrono::system_clock::time_point calendar_point::to_std_timepoint() const
   {
   if(get_year() < 1970)
      throw Invalid_Argument("calendar_point::to_std_timepoint() does not support years before 1970");

   // 32 bit time_t ends at January 19, 2038
   // https://msdn.microsoft.com/en-us/library/2093ets1.aspx
   // Throw after 2037 if 32 bit time_t is used
   if(get_year() > 2037 && sizeof(std::time_t) == 4)
      {
      throw Invalid_Argument("calendar_point::to_std_timepoint() does not support years after 2037 on this system");
      }
   else if(get_year() >= 2400)
      {
      // This upper bound is somewhat arbitrary
      throw Invalid_Argument("calendar_point::to_std_timepoint() does not support years after 2400");
      }

   const uint64_t seconds_64 = (days_since_epoch(get_year(), get_month(), get_day()) * 86400) +
                                (get_hour() * 60 * 60) + (get_minutes() * 60) + get_seconds();

   const time_t seconds_time_t = static_cast<time_t>(seconds_64);

   if(seconds_64 - seconds_time_t != 0)
      {
      throw Invalid_Argument("calendar_point::to_std_timepoint time_t overflow");
      }

   return std::chrono::system_clock::from_time_t(seconds_time_t);
   }

std::string calendar_point::to_string() const
   {
   // desired format: <YYYY>-<MM>-<dd>T<HH>:<mm>:<ss>
   std::stringstream output;
   output << std::setfill('0')
          << std::setw(4) << get_year() << "-"
          << std::setw(2) << get_month() << "-"
          << std::setw(2) << get_day() << "T"
          << std::setw(2) << get_hour() << ":"
          << std::setw(2) << get_minutes() << ":"
          << std::setw(2) << get_seconds();
   return output.str();
   }


calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point)
   {
   std::tm tm = do_gmtime(std::chrono::system_clock::to_time_t(time_point));

   return calendar_point(tm.tm_year + 1900,
                         tm.tm_mon + 1,
                         tm.tm_mday,
                         tm.tm_hour,
                         tm.tm_min,
                         tm.tm_sec);
   }

}
/*
* Character Set Handling
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <cctype>

namespace Botan {

namespace {

void append_utf8_for(std::string& s, uint32_t c)
   {
   if(c >= 0xD800 && c < 0xE000)
      throw Decoding_Error("Invalid Unicode character");

   if(c <= 0x7F)
      {
      const uint8_t b0 = static_cast<uint8_t>(c);
      s.push_back(static_cast<char>(b0));
      }
   else if(c <= 0x7FF)
      {
      const uint8_t b0 = 0xC0 | static_cast<uint8_t>(c >> 6);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
      }
   else if(c <= 0xFFFF)
      {
      const uint8_t b0 = 0xE0 | static_cast<uint8_t>(c >> 12);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>((c >> 6) & 0x3F);
      const uint8_t b2 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
      s.push_back(static_cast<char>(b2));
      }
   else if(c <= 0x10FFFF)
      {
      const uint8_t b0 = 0xF0 | static_cast<uint8_t>(c >> 18);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>((c >> 12) & 0x3F);
      const uint8_t b2 = 0x80 | static_cast<uint8_t>((c >> 6) & 0x3F);
      const uint8_t b3 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
      s.push_back(static_cast<char>(b2));
      s.push_back(static_cast<char>(b3));
      }
   else
      throw Decoding_Error("Invalid Unicode character");

   }

}

std::string ucs2_to_utf8(const uint8_t ucs2[], size_t len)
   {
   if(len % 2 != 0)
      throw Decoding_Error("Invalid length for UCS-2 string");

   const size_t chars = len / 2;

   std::string s;
   for(size_t i = 0; i != chars; ++i)
      {
      const uint16_t c = load_be<uint16_t>(ucs2, i);
      append_utf8_for(s, c);
      }

   return s;
   }

std::string ucs4_to_utf8(const uint8_t ucs4[], size_t len)
   {
   if(len % 4 != 0)
      throw Decoding_Error("Invalid length for UCS-4 string");

   const size_t chars = len / 4;

   std::string s;
   for(size_t i = 0; i != chars; ++i)
      {
      const uint32_t c = load_be<uint32_t>(ucs4, i);
      append_utf8_for(s, c);
      }

   return s;
   }

/*
* Convert from UTF-8 to ISO 8859-1
*/
std::string utf8_to_latin1(const std::string& utf8)
   {
   std::string iso8859;

   size_t position = 0;
   while(position != utf8.size())
      {
      const uint8_t c1 = static_cast<uint8_t>(utf8[position++]);

      if(c1 <= 0x7F)
         {
         iso8859 += static_cast<char>(c1);
         }
      else if(c1 >= 0xC0 && c1 <= 0xC7)
         {
         if(position == utf8.size())
            throw Decoding_Error("UTF-8: sequence truncated");

         const uint8_t c2 = static_cast<uint8_t>(utf8[position++]);
         const uint8_t iso_char = ((c1 & 0x07) << 6) | (c2 & 0x3F);

         if(iso_char <= 0x7F)
            throw Decoding_Error("UTF-8: sequence longer than needed");

         iso8859 += static_cast<char>(iso_char);
         }
      else
         throw Decoding_Error("UTF-8: Unicode chars not in Latin1 used");
      }

   return iso8859;
   }

namespace Charset {

namespace {

/*
* Convert from UCS-2 to ISO 8859-1
*/
std::string ucs2_to_latin1(const std::string& ucs2)
   {
   if(ucs2.size() % 2 == 1)
      throw Decoding_Error("UCS-2 string has an odd number of bytes");

   std::string latin1;

   for(size_t i = 0; i != ucs2.size(); i += 2)
      {
      const uint8_t c1 = ucs2[i];
      const uint8_t c2 = ucs2[i+1];

      if(c1 != 0)
         throw Decoding_Error("UCS-2 has non-Latin1 characters");

      latin1 += static_cast<char>(c2);
      }

   return latin1;
   }

/*
* Convert from ISO 8859-1 to UTF-8
*/
std::string latin1_to_utf8(const std::string& iso8859)
   {
   std::string utf8;
   for(size_t i = 0; i != iso8859.size(); ++i)
      {
      const uint8_t c = static_cast<uint8_t>(iso8859[i]);

      if(c <= 0x7F)
         utf8 += static_cast<char>(c);
      else
         {
         utf8 += static_cast<char>((0xC0 | (c >> 6)));
         utf8 += static_cast<char>((0x80 | (c & 0x3F)));
         }
      }
   return utf8;
   }

}

/*
* Perform character set transcoding
*/
std::string transcode(const std::string& str,
                      Character_Set to, Character_Set from)
   {
   if(to == LOCAL_CHARSET)
      to = LATIN1_CHARSET;
   if(from == LOCAL_CHARSET)
      from = LATIN1_CHARSET;

   if(to == from)
      return str;

   if(from == LATIN1_CHARSET && to == UTF8_CHARSET)
      return latin1_to_utf8(str);
   if(from == UTF8_CHARSET && to == LATIN1_CHARSET)
      return utf8_to_latin1(str);
   if(from == UCS2_CHARSET && to == LATIN1_CHARSET)
      return ucs2_to_latin1(str);

   throw Invalid_Argument("Unknown transcoding operation from " +
                          std::to_string(from) + " to " + std::to_string(to));
   }

/*
* Check if a character represents a digit
*/
bool is_digit(char c)
   {
   if(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
      c == '5' || c == '6' || c == '7' || c == '8' || c == '9')
      return true;
   return false;
   }

/*
* Check if a character represents whitespace
*/
bool is_space(char c)
   {
   if(c == ' ' || c == '\t' || c == '\n' || c == '\r')
      return true;
   return false;
   }

/*
* Convert a character to a digit
*/
uint8_t char2digit(char c)
   {
   switch(c)
      {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      }

   throw Invalid_Argument("char2digit: Input is not a digit character");
   }

/*
* Convert a digit to a character
*/
char digit2char(uint8_t b)
   {
   switch(b)
      {
      case 0: return '0';
      case 1: return '1';
      case 2: return '2';
      case 3: return '3';
      case 4: return '4';
      case 5: return '5';
      case 6: return '6';
      case 7: return '7';
      case 8: return '8';
      case 9: return '9';
      }

   throw Invalid_Argument("digit2char: Input is not a digit");
   }

/*
* Case-insensitive character comparison
*/
bool caseless_cmp(char a, char b)
   {
   return (std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b)));
   }

}

}
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace CT {

secure_vector<uint8_t> copy_output(CT::Mask<uint8_t> bad_input,
                                   const uint8_t input[],
                                   size_t input_length,
                                   size_t offset)
   {
   if(input_length == 0)
      return secure_vector<uint8_t>();

   /*
   * Ensure at runtime that offset <= input_length. This is an invalid input,
   * but we can't throw without using the poisoned value. Instead, if it happens,
   * set offset to be equal to the input length (so output_bytes becomes 0 and
   * the returned vector is empty)
   */
   const auto valid_offset = CT::Mask<size_t>::is_lte(offset, input_length);
   offset = valid_offset.select(offset, input_length);

   const size_t output_bytes = input_length - offset;

   secure_vector<uint8_t> output(input_length);

   /*
   Move the desired output bytes to the front using a slow (O^n)
   but constant time loop that does not leak the value of the offset
   */
   for(size_t i = 0; i != input_length; ++i)
      {
      /*
      start index from i rather than 0 since we know j must be >= i + offset
      to have any effect, and starting from i does not reveal information
      */
      for(size_t j = i; j != input_length; ++j)
         {
         const uint8_t b = input[j];
         const auto is_eq = CT::Mask<size_t>::is_equal(j, offset + i);
         output[i] |= is_eq.if_set_return(b);
         }
      }

   bad_input.if_set_zero_out(output.data(), output.size());

   /*
   This is potentially not const time, depending on how std::vector is
   implemented. But since we are always reducing length, it should
   just amount to setting the member var holding the length.
   */
   CT::unpoison(output.data(), output.size());
   CT::unpoison(output_bytes);
   output.resize(output_bytes);
   return output;
   }

secure_vector<uint8_t> strip_leading_zeros(const uint8_t in[], size_t length)
   {
   size_t leading_zeros = 0;

   auto only_zeros = Mask<uint8_t>::set();

   for(size_t i = 0; i != length; ++i)
      {
      only_zeros &= CT::Mask<uint8_t>::is_zero(in[i]);
      leading_zeros += only_zeros.if_set_return(1);
      }

   return copy_output(CT::Mask<uint8_t>::cleared(), in, length, leading_zeros);
   }

}

}
/*
* DataSource
* (C) 1999-2007 Jack Lloyd
*     2005 Matthew Gregan
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
  #include <fstream>
#endif

namespace Botan {

/*
* Read a single byte from the DataSource
*/
size_t DataSource::read_byte(uint8_t& out)
   {
   return read(&out, 1);
   }

/*
* Peek a single byte from the DataSource
*/
size_t DataSource::peek_byte(uint8_t& out) const
   {
   return peek(&out, 1, 0);
   }

/*
* Discard the next N bytes of the data
*/
size_t DataSource::discard_next(size_t n)
   {
   uint8_t buf[64] = { 0 };
   size_t discarded = 0;

   while(n)
      {
      const size_t got = this->read(buf, std::min(n, sizeof(buf)));
      discarded += got;
      n -= got;

      if(got == 0)
         break;
      }

   return discarded;
   }

/*
* Read from a memory buffer
*/
size_t DataSource_Memory::read(uint8_t out[], size_t length)
   {
   const size_t got = std::min<size_t>(m_source.size() - m_offset, length);
   copy_mem(out, m_source.data() + m_offset, got);
   m_offset += got;
   return got;
   }

bool DataSource_Memory::check_available(size_t n)
   {
   return (n <= (m_source.size() - m_offset));
   }

/*
* Peek into a memory buffer
*/
size_t DataSource_Memory::peek(uint8_t out[], size_t length,
                               size_t peek_offset) const
   {
   const size_t bytes_left = m_source.size() - m_offset;
   if(peek_offset >= bytes_left) return 0;

   const size_t got = std::min(bytes_left - peek_offset, length);
   copy_mem(out, &m_source[m_offset + peek_offset], got);
   return got;
   }

/*
* Check if the memory buffer is empty
*/
bool DataSource_Memory::end_of_data() const
   {
   return (m_offset == m_source.size());
   }

/*
* DataSource_Memory Constructor
*/
DataSource_Memory::DataSource_Memory(const std::string& in) :
   m_source(cast_char_ptr_to_uint8(in.data()),
            cast_char_ptr_to_uint8(in.data()) + in.length()),
   m_offset(0)
   {
   }

/*
* Read from a stream
*/
size_t DataSource_Stream::read(uint8_t out[], size_t length)
   {
   m_source.read(cast_uint8_ptr_to_char(out), length);
   if(m_source.bad())
      throw Stream_IO_Error("DataSource_Stream::read: Source failure");

   const size_t got = static_cast<size_t>(m_source.gcount());
   m_total_read += got;
   return got;
   }

bool DataSource_Stream::check_available(size_t n)
   {
   const std::streampos orig_pos = m_source.tellg();
   m_source.seekg(0, std::ios::end);
   const size_t avail = static_cast<size_t>(m_source.tellg() - orig_pos);
   m_source.seekg(orig_pos);
   return (avail >= n);
   }

/*
* Peek into a stream
*/
size_t DataSource_Stream::peek(uint8_t out[], size_t length, size_t offset) const
   {
   if(end_of_data())
      throw Invalid_State("DataSource_Stream: Cannot peek when out of data");

   size_t got = 0;

   if(offset)
      {
      secure_vector<uint8_t> buf(offset);
      m_source.read(cast_uint8_ptr_to_char(buf.data()), buf.size());
      if(m_source.bad())
         throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
      got = static_cast<size_t>(m_source.gcount());
      }

   if(got == offset)
      {
      m_source.read(cast_uint8_ptr_to_char(out), length);
      if(m_source.bad())
         throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
      got = static_cast<size_t>(m_source.gcount());
      }

   if(m_source.eof())
      m_source.clear();
   m_source.seekg(m_total_read, std::ios::beg);

   return got;
   }

/*
* Check if the stream is empty or in error
*/
bool DataSource_Stream::end_of_data() const
   {
   return (!m_source.good());
   }

/*
* Return a human-readable ID for this stream
*/
std::string DataSource_Stream::id() const
   {
   return m_identifier;
   }

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(const std::string& path,
                                     bool use_binary) :
   m_identifier(path),
   m_source_memory(new std::ifstream(path, use_binary ? std::ios::binary : std::ios::in)),
   m_source(*m_source_memory),
   m_total_read(0)
   {
   if(!m_source.good())
      {
      throw Stream_IO_Error("DataSource: Failure opening file " + path);
      }
   }

#endif

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(std::istream& in,
                                     const std::string& name) :
   m_identifier(name),
   m_source(in),
   m_total_read(0)
   {
   }

DataSource_Stream::~DataSource_Stream()
   {
   // for ~unique_ptr
   }

}
/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

Exception::Exception(const std::string& msg) : m_msg(msg)
   {}

Exception::Exception(const std::string& msg, const std::exception& e) :
   m_msg(msg + " failed with " + std::string(e.what()))
   {}

Exception::Exception(const char* prefix, const std::string& msg) :
   m_msg(std::string(prefix) + " " + msg)
   {}

Invalid_Argument::Invalid_Argument(const std::string& msg) :
   Exception(msg)
   {}

Invalid_Argument::Invalid_Argument(const std::string& msg, const std::string& where) :
   Exception(msg + " in " + where)
   {}

Invalid_Argument::Invalid_Argument(const std::string& msg, const std::exception& e) :
   Exception(msg, e) {}

Lookup_Error::Lookup_Error(const std::string& type,
                           const std::string& algo,
                           const std::string& provider) :
   Exception("Unavailable " + type + " " + algo +
             (provider.empty() ? std::string("") : (" for provider " + provider)))
   {}

Internal_Error::Internal_Error(const std::string& err) :
   Exception("Internal error: " + err)
   {}

Invalid_Key_Length::Invalid_Key_Length(const std::string& name, size_t length) :
   Invalid_Argument(name + " cannot accept a key of length " +
                    std::to_string(length))
   {}

Invalid_IV_Length::Invalid_IV_Length(const std::string& mode, size_t bad_len) :
   Invalid_Argument("IV length " + std::to_string(bad_len) +
                    " is invalid for " + mode)
   {}

Key_Not_Set::Key_Not_Set(const std::string& algo) :
   Invalid_State("Key not set in " + algo)
   {}

Policy_Violation::Policy_Violation(const std::string& err) :
   Invalid_State("Policy violation: " + err) {}

PRNG_Unseeded::PRNG_Unseeded(const std::string& algo) :
   Invalid_State("PRNG not seeded: " + algo)
   {}

Algorithm_Not_Found::Algorithm_Not_Found(const std::string& name) :
   Lookup_Error("Could not find any algorithm named \"" + name + "\"")
   {}

No_Provider_Found::No_Provider_Found(const std::string& name) :
   Exception("Could not find any provider for algorithm named \"" + name + "\"")
   {}

Provider_Not_Found::Provider_Not_Found(const std::string& algo, const std::string& provider) :
   Lookup_Error("Could not find provider '" + provider + "' for " + algo)
   {}

Invalid_Algorithm_Name::Invalid_Algorithm_Name(const std::string& name):
   Invalid_Argument("Invalid algorithm name: " + name)
   {}

Encoding_Error::Encoding_Error(const std::string& name) :
   Invalid_Argument("Encoding error: " + name)
   {}

Decoding_Error::Decoding_Error(const std::string& name) :
   Invalid_Argument(name)
   {}

Decoding_Error::Decoding_Error(const std::string& msg, const std::exception& e) :
   Invalid_Argument(msg, e)
   {}

Decoding_Error::Decoding_Error(const std::string& name, const char* exception_message) :
   Invalid_Argument(name + " failed with exception " + exception_message) {}

Integrity_Failure::Integrity_Failure(const std::string& msg) :
   Exception("Integrity failure: " + msg)
   {}

Invalid_OID::Invalid_OID(const std::string& oid) :
   Decoding_Error("Invalid ASN.1 OID: " + oid)
   {}

Stream_IO_Error::Stream_IO_Error(const std::string& err) :
   Exception("I/O error: " + err)
   {}

System_Error::System_Error(const std::string& msg, int err_code) :
   Exception(msg + " error code " + std::to_string(err_code)),
   m_error_code(err_code)
   {}

Self_Test_Failure::Self_Test_Failure(const std::string& err) :
   Internal_Error("Self test failed: " + err)
   {}

Not_Implemented::Not_Implemented(const std::string& err) :
   Exception("Not implemented", err)
   {}

}
/*
* (C) 2015,2017 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
  #include <filesystem>
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
  #include <boost/filesystem.hpp>
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
  #define NOMINMAX 1
  #define _WINSOCKAPI_ // stop windows.h including winsock.h
  #include <windows.h>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
std::vector<std::string> impl_stl_filesystem(const std::string& dir)
   {
#if (_MSVC_LANG >= 201703L)
   using namespace std::filesystem;
#else
   using namespace std::tr2::sys;
#endif

   std::vector<std::string> out;

   path p(dir);

   if(is_directory(p))
      {
      for(recursive_directory_iterator itr(p), end; itr != end; ++itr)
         {
         if(is_regular_file(itr->path()))
            {
            out.push_back(itr->path().string());
            }
         }
      }

   return out;
   }

#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)

std::vector<std::string> impl_boost_filesystem(const std::string& dir_path)
{
   namespace fs = boost::filesystem;

   std::vector<std::string> out;

   for(fs::recursive_directory_iterator dir(dir_path), end; dir != end; ++dir)
      {
      if(fs::is_regular_file(dir->path()))
         {
         out.push_back(dir->path().string());
         }
      }

   return out;
}

#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)

std::vector<std::string> impl_readdir(const std::string& dir_path)
   {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(dir_path);

   while(!dir_list.empty())
      {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      std::unique_ptr<DIR, std::function<int (DIR*)>> dir(::opendir(cur_path.c_str()), ::closedir);

      if(dir)
         {
         while(struct dirent* dirent = ::readdir(dir.get()))
            {
            const std::string filename = dirent->d_name;
            if(filename == "." || filename == "..")
               continue;
            const std::string full_path = cur_path + "/" + filename;

            struct stat stat_buf;

            if(::stat(full_path.c_str(), &stat_buf) == -1)
               continue;

            if(S_ISDIR(stat_buf.st_mode))
               dir_list.push_back(full_path);
            else if(S_ISREG(stat_buf.st_mode))
               out.push_back(full_path);
            }
         }
      }

   return out;
   }

#elif defined(BOTAN_TARGET_OS_HAS_WIN32)

std::vector<std::string> impl_win32(const std::string& dir_path)
   {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(dir_path);

   while(!dir_list.empty())
      {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      WIN32_FIND_DATAA find_data;
      HANDLE dir = ::FindFirstFileA((cur_path + "/*").c_str(), &find_data);

      if(dir != INVALID_HANDLE_VALUE)
         {
         do
            {
            const std::string filename = find_data.cFileName;
            if(filename == "." || filename == "..")
               continue;
            const std::string full_path = cur_path + "/" + filename;

            if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
               {
               dir_list.push_back(full_path);
               }
            else
               {
               out.push_back(full_path);
               }
            }
         while(::FindNextFileA(dir, &find_data));
         }

      ::FindClose(dir);
      }

   return out;
}
#endif

}

bool has_filesystem_impl()
   {
#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   return true;
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
   return true;
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return true;
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   return true;
#else
   return false;
#endif
   }

std::vector<std::string> get_files_recursive(const std::string& dir)
   {
   std::vector<std::string> files;

#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   files = impl_stl_filesystem(dir);
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
   files = impl_boost_filesystem(dir);
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   files = impl_readdir(dir);
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   files = impl_win32(dir);
#else
   BOTAN_UNUSED(dir);
   throw No_Filesystem_Access();
#endif

   std::sort(files.begin(), files.end());

   return files;
   }

}
/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <cstdlib>
#include <new>

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
#endif

namespace Botan {

BOTAN_MALLOC_FN void* allocate_memory(size_t elems, size_t elem_size)
   {
#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   if(void* p = mlock_allocator::instance().allocate(elems, elem_size))
      return p;
#endif

   void* ptr = std::calloc(elems, elem_size);
   if(!ptr)
      throw std::bad_alloc();
   return ptr;
   }

void deallocate_memory(void* p, size_t elems, size_t elem_size)
   {
   if(p == nullptr)
      return;

   secure_scrub_memory(p, elems * elem_size);

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   if(mlock_allocator::instance().deallocate(p, elems, elem_size))
      return;
#endif

   std::free(p);
   }

void initialize_allocator()
   {
#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   mlock_allocator::instance();
#endif
   }

uint8_t ct_compare_u8(const uint8_t x[],
                      const uint8_t y[],
                      size_t len)
   {
   volatile uint8_t difference = 0;

   for(size_t i = 0; i != len; ++i)
      difference |= (x[i] ^ y[i]);

   return CT::Mask<uint8_t>::is_zero(difference).value();
   }

}
/*
* OS and machine specific utility functions
* (C) 2015,2016,2017,2018 Jack Lloyd
* (C) 2016 Daniel Neus
*
* Botan is released under the Simplified BSD License (see license.txt)
*/



#if defined(BOTAN_TARGET_OS_HAS_EXPLICIT_BZERO)
  #include <string.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
  #include <sys/types.h>
  #include <sys/resource.h>
  #include <sys/mman.h>
  #include <signal.h>
  #include <setjmp.h>
  #include <unistd.h>
  #include <errno.h>
  #include <termios.h>
  #undef B0
#endif

#if defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
  #include <emscripten/emscripten.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL)
  #include <sys/auxv.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
  #define NOMINMAX 1
  #include <windows.h>
#endif

namespace Botan {

// Not defined in OS namespace for historical reasons
void secure_scrub_memory(void* ptr, size_t n)
   {
#if defined(BOTAN_TARGET_OS_HAS_RTLSECUREZEROMEMORY)
   ::RtlSecureZeroMemory(ptr, n);

#elif defined(BOTAN_TARGET_OS_HAS_EXPLICIT_BZERO)
   ::explicit_bzero(ptr, n);

#elif defined(BOTAN_TARGET_OS_HAS_EXPLICIT_MEMSET)
   (void)::explicit_memset(ptr, 0, n);

#elif defined(BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO) && (BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO == 1)
   /*
   Call memset through a static volatile pointer, which the compiler
   should not elide. This construct should be safe in conforming
   compilers, but who knows. I did confirm that on x86-64 GCC 6.1 and
   Clang 3.8 both create code that saves the memset address in the
   data segment and unconditionally loads and jumps to that address.
   */
   static void* (*const volatile memset_ptr)(void*, int, size_t) = std::memset;
   (memset_ptr)(ptr, 0, n);
#else

   volatile uint8_t* p = reinterpret_cast<volatile uint8_t*>(ptr);

   for(size_t i = 0; i != n; ++i)
      p[i] = 0;
#endif
   }

uint32_t OS::get_process_id()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return ::getpid();
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   return ::GetCurrentProcessId();
#elif defined(BOTAN_TARGET_OS_IS_INCLUDEOS) || defined(BOTAN_TARGET_OS_IS_LLVM)
   return 0; // truly no meaningful value
#else
   #error "Missing get_process_id"
#endif
   }

bool OS::running_in_privileged_state()
   {
#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL) && defined(AT_SECURE)
   return ::getauxval(AT_SECURE) != 0;
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return (::getuid() != ::geteuid()) || (::getgid() != ::getegid());
#else
   return false;
#endif
   }

uint64_t OS::get_cpu_cycle_counter()
   {
   uint64_t rtc = 0;

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   LARGE_INTEGER tv;
   ::QueryPerformanceCounter(&tv);
   rtc = tv.QuadPart;

#elif defined(BOTAN_USE_GCC_INLINE_ASM)

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

   if(CPUID::has_rdtsc())
      {
      uint32_t rtc_low = 0, rtc_high = 0;
      asm volatile("rdtsc" : "=d" (rtc_high), "=a" (rtc_low));
      rtc = (static_cast<uint64_t>(rtc_high) << 32) | rtc_low;
      }

#elif defined(BOTAN_TARGET_ARCH_IS_PPC64)

   for(;;)
      {
      uint32_t rtc_low = 0, rtc_high = 0, rtc_high2 = 0;
      asm volatile("mftbu %0" : "=r" (rtc_high));
      asm volatile("mftb %0" : "=r" (rtc_low));
      asm volatile("mftbu %0" : "=r" (rtc_high2));

      if(rtc_high == rtc_high2)
	 {
         rtc = (static_cast<uint64_t>(rtc_high) << 32) | rtc_low;
         break;
	 }
      }

#elif defined(BOTAN_TARGET_ARCH_IS_ALPHA)
   asm volatile("rpcc %0" : "=r" (rtc));

   // OpenBSD does not trap access to the %tick register
#elif defined(BOTAN_TARGET_ARCH_IS_SPARC64) && !defined(BOTAN_TARGET_OS_IS_OPENBSD)
   asm volatile("rd %%tick, %0" : "=r" (rtc));

#elif defined(BOTAN_TARGET_ARCH_IS_IA64)
   asm volatile("mov %0=ar.itc" : "=r" (rtc));

#elif defined(BOTAN_TARGET_ARCH_IS_S390X)
   asm volatile("stck 0(%0)" : : "a" (&rtc) : "memory", "cc");

#elif defined(BOTAN_TARGET_ARCH_IS_HPPA)
   asm volatile("mfctl 16,%0" : "=r" (rtc)); // 64-bit only?

#else
   //#warning "OS::get_cpu_cycle_counter not implemented"
#endif

#endif

   return rtc;
   }

uint64_t OS::get_high_resolution_clock()
   {
   if(uint64_t cpu_clock = OS::get_cpu_cycle_counter())
      return cpu_clock;

#if defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
   return emscripten_get_now();
#endif

   /*
   If we got here either we either don't have an asm instruction
   above, or (for x86) RDTSC is not available at runtime. Try some
   clock_gettimes and return the first one that works, or otherwise
   fall back to std::chrono.
   */

#if defined(BOTAN_TARGET_OS_HAS_CLOCK_GETTIME)

   // The ordering here is somewhat arbitrary...
   const clockid_t clock_types[] = {
#if defined(CLOCK_MONOTONIC_HR)
      CLOCK_MONOTONIC_HR,
#endif
#if defined(CLOCK_MONOTONIC_RAW)
      CLOCK_MONOTONIC_RAW,
#endif
#if defined(CLOCK_MONOTONIC)
      CLOCK_MONOTONIC,
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
      CLOCK_PROCESS_CPUTIME_ID,
#endif
#if defined(CLOCK_THREAD_CPUTIME_ID)
      CLOCK_THREAD_CPUTIME_ID,
#endif
   };

   for(clockid_t clock : clock_types)
      {
      struct timespec ts;
      if(::clock_gettime(clock, &ts) == 0)
         {
         return (static_cast<uint64_t>(ts.tv_sec) * 1000000000) + static_cast<uint64_t>(ts.tv_nsec);
         }
      }
#endif

   // Plain C++11 fallback
   auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
   return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
   }

uint64_t OS::get_system_timestamp_ns()
   {
#if defined(BOTAN_TARGET_OS_HAS_CLOCK_GETTIME)
   struct timespec ts;
   if(::clock_gettime(CLOCK_REALTIME, &ts) == 0)
      {
      return (static_cast<uint64_t>(ts.tv_sec) * 1000000000) + static_cast<uint64_t>(ts.tv_nsec);
      }
#endif

   auto now = std::chrono::system_clock::now().time_since_epoch();
   return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
   }

size_t OS::system_page_size()
   {
   const size_t default_page_size = 4096;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   long p = ::sysconf(_SC_PAGESIZE);
   if(p > 1)
      return static_cast<size_t>(p);
   else
      return default_page_size;
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   SYSTEM_INFO sys_info;
   ::GetSystemInfo(&sys_info);
   return sys_info.dwPageSize;
#else
   return default_page_size;
#endif
   }

size_t OS::get_memory_locking_limit()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK) && defined(RLIMIT_MEMLOCK)
   /*
   * If RLIMIT_MEMLOCK is not defined, likely the OS does not support
   * unprivileged mlock calls.
   *
   * Linux defaults to only 64 KiB of mlockable memory per process
   * (too small) but BSDs offer a small fraction of total RAM (more
   * than we need). Bound the total mlock size to 512 KiB which is
   * enough to run the entire test suite without spilling to non-mlock
   * memory (and thus presumably also enough for many useful
   * programs), but small enough that we should not cause problems
   * even if many processes are mlocking on the same machine.
   */
   size_t mlock_requested = BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB;

   /*
   * Allow override via env variable
   */
   if(const char* env = read_env_variable("BOTAN_MLOCK_POOL_SIZE"))
      {
      try
         {
         const size_t user_req = std::stoul(env, nullptr);
         mlock_requested = std::min(user_req, mlock_requested);
         }
      catch(std::exception&) { /* ignore it */ }
      }

   if(mlock_requested > 0)
      {
      struct ::rlimit limits;

      ::getrlimit(RLIMIT_MEMLOCK, &limits);

      if(limits.rlim_cur < limits.rlim_max)
         {
         limits.rlim_cur = limits.rlim_max;
         ::setrlimit(RLIMIT_MEMLOCK, &limits);
         ::getrlimit(RLIMIT_MEMLOCK, &limits);
         }

      return std::min<size_t>(limits.rlim_cur, mlock_requested * 1024);
      }

#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   SIZE_T working_min = 0, working_max = 0;
   if(!::GetProcessWorkingSetSize(::GetCurrentProcess(), &working_min, &working_max))
      {
      return 0;
      }

   // According to Microsoft MSDN:
   // The maximum number of pages that a process can lock is equal to the number of pages in its minimum working set minus a small overhead
   // In the book "Windows Internals Part 2": the maximum lockable pages are minimum working set size - 8 pages 
   // But the information in the book seems to be inaccurate/outdated
   // I've tested this on Windows 8.1 x64, Windows 10 x64 and Windows 7 x86
   // On all three OS the value is 11 instead of 8
   size_t overhead = OS::system_page_size() * 11ULL;
   if(working_min > overhead)
      {
      size_t lockable_bytes = working_min - overhead;
      if(lockable_bytes < (BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB * 1024ULL))
         {
         return lockable_bytes;
         }
      else
         {
         return BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB * 1024ULL;
         }
      }
#endif

   // Not supported on this platform
   return 0;
   }

const char* OS::read_env_variable(const std::string& name)
   {
   if(running_in_privileged_state())
      return nullptr;

   return std::getenv(name.c_str());
   }

void* OS::allocate_locked_pages(size_t length)
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)

   const size_t page_size = OS::system_page_size();

   if(length % page_size != 0)
      return nullptr;

   void* ptr = nullptr;
   int rc = ::posix_memalign(&ptr, page_size, length);

   if(rc != 0 || ptr == nullptr)
      return nullptr;

#if defined(MADV_DONTDUMP)
   ::madvise(ptr, length, MADV_DONTDUMP);
#endif

   if(::mlock(ptr, length) != 0)
      {
      std::free(ptr);
      return nullptr; // failed to lock
      }

   ::memset(ptr, 0, length);

   return ptr;
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   LPVOID ptr = ::VirtualAlloc(nullptr, length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
   if(!ptr)
      {
      return nullptr;
      }

   if(::VirtualLock(ptr, length) == 0)
      {
      ::VirtualFree(ptr, 0, MEM_RELEASE);
      return nullptr; // failed to lock
      }

   return ptr;
#else
   BOTAN_UNUSED(length);
   return nullptr; /* not implemented */
#endif
   }

void OS::free_locked_pages(void* ptr, size_t length)
   {
   if(ptr == nullptr || length == 0)
      return;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
   secure_scrub_memory(ptr, length);
   ::munlock(ptr, length);
   std::free(ptr);

#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   secure_scrub_memory(ptr, length);
   ::VirtualUnlock(ptr, length);
   ::VirtualFree(ptr, 0, MEM_RELEASE);

#else
   // Invalid argument because no way this pointer was allocated by us
   throw Invalid_Argument("Invalid ptr to free_locked_pages");
#endif
   }

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && !defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)

namespace {

static ::sigjmp_buf g_sigill_jmp_buf;

void botan_sigill_handler(int)
   {
   siglongjmp(g_sigill_jmp_buf, /*non-zero return value*/1);
   }

}

#endif

int OS::run_cpu_instruction_probe(std::function<int ()> probe_fn)
   {
   volatile int probe_result = -3;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && !defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
   struct sigaction old_sigaction;
   struct sigaction sigaction;

   sigaction.sa_handler = botan_sigill_handler;
   sigemptyset(&sigaction.sa_mask);
   sigaction.sa_flags = 0;

   int rc = ::sigaction(SIGILL, &sigaction, &old_sigaction);

   if(rc != 0)
      throw System_Error("run_cpu_instruction_probe sigaction failed", errno);

   rc = sigsetjmp(g_sigill_jmp_buf, /*save sigs*/1);

   if(rc == 0)
      {
      // first call to sigsetjmp
      probe_result = probe_fn();
      }
   else if(rc == 1)
      {
      // non-local return from siglongjmp in signal handler: return error
      probe_result = -1;
      }

   // Restore old SIGILL handler, if any
   rc = ::sigaction(SIGILL, &old_sigaction, nullptr);
   if(rc != 0)
      throw System_Error("run_cpu_instruction_probe sigaction restore failed", errno);

#elif defined(BOTAN_TARGET_OS_IS_WINDOWS) && defined(BOTAN_TARGET_COMPILER_IS_MSVC)

   // Windows SEH
   __try
      {
      probe_result = probe_fn();
      }
   __except(::GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
      {
      probe_result = -1;
      }

#else
   BOTAN_UNUSED(probe_fn);
#endif

   return probe_result;
   }

std::unique_ptr<OS::Echo_Suppression> OS::suppress_echo_on_terminal()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   class POSIX_Echo_Suppression : public Echo_Suppression
      {
      public:
         POSIX_Echo_Suppression()
            {
            m_stdin_fd = fileno(stdin);
            if(::tcgetattr(m_stdin_fd, &m_old_termios) != 0)
               throw System_Error("Getting terminal status failed", errno);

            struct termios noecho_flags = m_old_termios;
            noecho_flags.c_lflag &= ~ECHO;
            noecho_flags.c_lflag |= ECHONL;

            if(::tcsetattr(m_stdin_fd, TCSANOW, &noecho_flags) != 0)
               throw System_Error("Clearing terminal echo bit failed", errno);
            }

         void reenable_echo() override
            {
            if(m_stdin_fd > 0)
               {
               if(::tcsetattr(m_stdin_fd, TCSANOW, &m_old_termios) != 0)
                  throw System_Error("Restoring terminal echo bit failed", errno);
               m_stdin_fd = -1;
               }
            }

         ~POSIX_Echo_Suppression()
            {
            try
               {
               reenable_echo();
               }
            catch(...)
               {
               }
            }

      private:
         int m_stdin_fd;
         struct termios m_old_termios;
      };

   return std::unique_ptr<Echo_Suppression>(new POSIX_Echo_Suppression);

#elif defined(BOTAN_TARGET_OS_HAS_WIN32)

   class Win32_Echo_Suppression : public Echo_Suppression
      {
      public:
         Win32_Echo_Suppression()
            {
            m_input_handle = ::GetStdHandle(STD_INPUT_HANDLE);
            if(::GetConsoleMode(m_input_handle, &m_console_state) == 0)
               throw System_Error("Getting console mode failed", ::GetLastError());

            DWORD new_mode = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT;
            if(::SetConsoleMode(m_input_handle, new_mode) == 0)
               throw System_Error("Setting console mode failed", ::GetLastError());
            }

         void reenable_echo() override
            {
            if(m_input_handle != INVALID_HANDLE_VALUE)
               {
               if(::SetConsoleMode(m_input_handle, m_console_state) == 0)
                  throw System_Error("Setting console mode failed", ::GetLastError());
               m_input_handle = INVALID_HANDLE_VALUE;
               }
            }

         ~Win32_Echo_Suppression()
            {
            try
               {
               reenable_echo();
               }
            catch(...)
               {
               }
            }

      private:
         HANDLE m_input_handle;
         DWORD m_console_state;
      };

   return std::unique_ptr<Echo_Suppression>(new Win32_Echo_Suppression);

#else

   // Not supported on this platform, return null
   return std::unique_ptr<Echo_Suppression>();
#endif
   }

}
/*
* Various string utils and parsing functions
* (C) 1999-2007,2013,2014,2015,2018 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
* (C) 2017 René Korthaus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <limits>

namespace Botan {

uint16_t to_uint16(const std::string& str)
   {
   const uint32_t x = to_u32bit(str);

   if(x >> 16)
      throw Invalid_Argument("Integer value exceeds 16 bit range");

   return static_cast<uint16_t>(x);
   }

uint32_t to_u32bit(const std::string& str)
   {
   // std::stoul is not strict enough. Ensure that str is digit only [0-9]*
   for(const char chr : str)
      {
      if(chr < '0' || chr > '9')
         {
         std::string chrAsString(1, chr);
         throw Invalid_Argument("String contains non-digit char: " + chrAsString);
         }
      }

   const unsigned long int x = std::stoul(str);

   if(sizeof(unsigned long int) > 4)
      {
      // x might be uint64
      if (x > std::numeric_limits<uint32_t>::max())
         {
         throw Invalid_Argument("Integer value of " + str + " exceeds 32 bit range");
         }
      }

   return static_cast<uint32_t>(x);
   }

/*
* Convert a string into a time duration
*/
uint32_t timespec_to_u32bit(const std::string& timespec)
   {
   if(timespec.empty())
      return 0;

   const char suffix = timespec[timespec.size()-1];
   std::string value = timespec.substr(0, timespec.size()-1);

   uint32_t scale = 1;

   if(Charset::is_digit(suffix))
      value += suffix;
   else if(suffix == 's')
      scale = 1;
   else if(suffix == 'm')
      scale = 60;
   else if(suffix == 'h')
      scale = 60 * 60;
   else if(suffix == 'd')
      scale = 24 * 60 * 60;
   else if(suffix == 'y')
      scale = 365 * 24 * 60 * 60;
   else
      throw Decoding_Error("timespec_to_u32bit: Bad input " + timespec);

   return scale * to_u32bit(value);
   }

/*
* Parse a SCAN-style algorithm name
*/
std::vector<std::string> parse_algorithm_name(const std::string& namex)
   {
   if(namex.find('(') == std::string::npos &&
      namex.find(')') == std::string::npos)
      return std::vector<std::string>(1, namex);

   std::string name = namex, substring;
   std::vector<std::string> elems;
   size_t level = 0;

   elems.push_back(name.substr(0, name.find('(')));
   name = name.substr(name.find('('));

   for(auto i = name.begin(); i != name.end(); ++i)
      {
      char c = *i;

      if(c == '(')
         ++level;
      if(c == ')')
         {
         if(level == 1 && i == name.end() - 1)
            {
            if(elems.size() == 1)
               elems.push_back(substring.substr(1));
            else
               elems.push_back(substring);
            return elems;
            }

         if(level == 0 || (level == 1 && i != name.end() - 1))
            throw Invalid_Algorithm_Name(namex);
         --level;
         }

      if(c == ',' && level == 1)
         {
         if(elems.size() == 1)
            elems.push_back(substring.substr(1));
         else
            elems.push_back(substring);
         substring.clear();
         }
      else
         substring += c;
      }

   if(!substring.empty())
      throw Invalid_Algorithm_Name(namex);

   return elems;
   }

std::vector<std::string> split_on(const std::string& str, char delim)
   {
   return split_on_pred(str, [delim](char c) { return c == delim; });
   }

std::vector<std::string> split_on_pred(const std::string& str,
                                       std::function<bool (char)> pred)
   {
   std::vector<std::string> elems;
   if(str.empty()) return elems;

   std::string substr;
   for(auto i = str.begin(); i != str.end(); ++i)
      {
      if(pred(*i))
         {
         if(!substr.empty())
            elems.push_back(substr);
         substr.clear();
         }
      else
         substr += *i;
      }

   if(substr.empty())
      throw Invalid_Argument("Unable to split string: " + str);
   elems.push_back(substr);

   return elems;
   }

/*
* Join a string
*/
std::string string_join(const std::vector<std::string>& strs, char delim)
   {
   std::string out = "";

   for(size_t i = 0; i != strs.size(); ++i)
      {
      if(i != 0)
         out += delim;
      out += strs[i];
      }

   return out;
   }

/*
* Parse an ASN.1 OID string
*/
std::vector<uint32_t> parse_asn1_oid(const std::string& oid)
   {
   std::string substring;
   std::vector<uint32_t> oid_elems;

   for(auto i = oid.begin(); i != oid.end(); ++i)
      {
      char c = *i;

      if(c == '.')
         {
         if(substring.empty())
            throw Invalid_OID(oid);
         oid_elems.push_back(to_u32bit(substring));
         substring.clear();
         }
      else
         substring += c;
      }

   if(substring.empty())
      throw Invalid_OID(oid);
   oid_elems.push_back(to_u32bit(substring));

   if(oid_elems.size() < 2)
      throw Invalid_OID(oid);

   return oid_elems;
   }

/*
* X.500 String Comparison
*/
bool x500_name_cmp(const std::string& name1, const std::string& name2)
   {
   auto p1 = name1.begin();
   auto p2 = name2.begin();

   while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
   while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

   while(p1 != name1.end() && p2 != name2.end())
      {
      if(Charset::is_space(*p1))
         {
         if(!Charset::is_space(*p2))
            return false;

         while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
         while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

         if(p1 == name1.end() && p2 == name2.end())
            return true;
         if(p1 == name1.end() || p2 == name2.end())
            return false;
         }

      if(!Charset::caseless_cmp(*p1, *p2))
         return false;
      ++p1;
      ++p2;
      }

   while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
   while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

   if((p1 != name1.end()) || (p2 != name2.end()))
      return false;
   return true;
   }

/*
* Convert a decimal-dotted string to binary IP
*/
uint32_t string_to_ipv4(const std::string& str)
   {
   std::vector<std::string> parts = split_on(str, '.');

   if(parts.size() != 4)
      throw Decoding_Error("Invalid IP string " + str);

   uint32_t ip = 0;

   for(auto part = parts.begin(); part != parts.end(); ++part)
      {
      uint32_t octet = to_u32bit(*part);

      if(octet > 255)
         throw Decoding_Error("Invalid IP string " + str);

      ip = (ip << 8) | (octet & 0xFF);
      }

   return ip;
   }

/*
* Convert an IP address to decimal-dotted string
*/
std::string ipv4_to_string(uint32_t ip)
   {
   std::string str;

   for(size_t i = 0; i != sizeof(ip); ++i)
      {
      if(i)
         str += ".";
      str += std::to_string(get_byte(i, ip));
      }

   return str;
   }

std::string erase_chars(const std::string& str, const std::set<char>& chars)
   {
   std::string out;

   for(auto c: str)
      if(chars.count(c) == 0)
         out += c;

   return out;
   }

std::string replace_chars(const std::string& str,
                          const std::set<char>& chars,
                          char to_char)
   {
   std::string out = str;

   for(size_t i = 0; i != out.size(); ++i)
      if(chars.count(out[i]))
         out[i] = to_char;

   return out;
   }

std::string replace_char(const std::string& str, char from_char, char to_char)
   {
   std::string out = str;

   for(size_t i = 0; i != out.size(); ++i)
      if(out[i] == from_char)
         out[i] = to_char;

   return out;
   }

namespace {

std::string tolower_string(const std::string& in)
   {
   std::string s = in;
   for(size_t i = 0; i != s.size(); ++i)
      {
      const int cu = static_cast<unsigned char>(s[i]);
      if(std::isalpha(cu))
         s[i] = static_cast<char>(std::tolower(cu));
      }
   return s;
   }

}

bool host_wildcard_match(const std::string& issued_, const std::string& host_)
   {
   const std::string issued = tolower_string(issued_);
   const std::string host = tolower_string(host_);

   if(host.empty() || issued.empty())
      return false;

   /*
   If there are embedded nulls in your issued name
   Well I feel bad for you son
   */
   if(std::count(issued.begin(), issued.end(), char(0)) > 0)
      return false;

   // If more than one wildcard, then issued name is invalid
   const size_t stars = std::count(issued.begin(), issued.end(), '*');
   if(stars > 1)
      return false;

   // '*' is not a valid character in DNS names so should not appear on the host side
   if(std::count(host.begin(), host.end(), '*') != 0)
      return false;

   // Similarly a DNS name can't end in .
   if(host[host.size() - 1] == '.')
      return false;

   // And a host can't have an empty name component, so reject that
   if(host.find("..") != std::string::npos)
      return false;

   // Exact match: accept
   if(issued == host)
      {
      return true;
      }

   /*
   Otherwise it might be a wildcard

   If the issued size is strictly longer than the hostname size it
   couldn't possibly be a match, even if the issued value is a
   wildcard. The only exception is when the wildcard ends up empty
   (eg www.example.com matches www*.example.com)
   */
   if(issued.size() > host.size() + 1)
      {
      return false;
      }

   // If no * at all then not a wildcard, and so not a match
   if(stars != 1)
      {
      return false;
      }

   /*
   Now walk through the issued string, making sure every character
   matches. When we come to the (singular) '*', jump forward in the
   hostname by the corresponding amount. We know exactly how much
   space the wildcard takes because it must be exactly `len(host) -
   len(issued) + 1 chars`.

   We also verify that the '*' comes in the leftmost component, and
   doesn't skip over any '.' in the hostname.
   */
   size_t dots_seen = 0;
   size_t host_idx = 0;

   for(size_t i = 0; i != issued.size(); ++i)
      {
      dots_seen += (issued[i] == '.');

      if(issued[i] == '*')
         {
         // Fail: wildcard can only come in leftmost component
         if(dots_seen > 0)
            {
            return false;
            }

         /*
         Since there is only one * we know the tail of the issued and
         hostname must be an exact match. In this case advance host_idx
         to match.
         */
         const size_t advance = (host.size() - issued.size() + 1);

         if(host_idx + advance > host.size()) // shouldn't happen
            return false;

         // Can't be any intervening .s that we would have skipped
         if(std::count(host.begin() + host_idx,
                       host.begin() + host_idx + advance, '.') != 0)
            return false;

         host_idx += advance;
         }
      else
         {
         if(issued[i] != host[host_idx])
            {
            return false;
            }

         host_idx += 1;
         }
      }

   // Wildcard issued name must have at least 3 components
   if(dots_seen < 2)
      {
      return false;
      }

   return true;
   }

}
/*
* Simple config/test file reader
* (C) 2013,2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

std::string clean_ws(const std::string& s)
   {
   const char* ws = " \t\n";
   auto start = s.find_first_not_of(ws);
   auto end = s.find_last_not_of(ws);

   if(start == std::string::npos)
      return "";

   if(end == std::string::npos)
      return s.substr(start, end);
   else
      return s.substr(start, start + end + 1);
   }

std::map<std::string, std::string> read_cfg(std::istream& is)
   {
   std::map<std::string, std::string> kv;
   size_t line = 0;

   while(is.good())
      {
      std::string s;

      std::getline(is, s);

      ++line;

      if(s.empty() || s[0] == '#')
         continue;

      s = clean_ws(s.substr(0, s.find('#')));

      if(s.empty())
         continue;

      auto eq = s.find("=");

      if(eq == std::string::npos || eq == 0 || eq == s.size() - 1)
         throw Decoding_Error("Bad read_cfg input '" + s + "' on line " + std::to_string(line));

      const std::string key = clean_ws(s.substr(0, eq));
      const std::string val = clean_ws(s.substr(eq + 1, std::string::npos));

      kv[key] = val;
      }

   return kv;
   }

}
/*
* (C) 2018 Ribose Inc
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

std::map<std::string, std::string> read_kv(const std::string& kv)
   {
   std::map<std::string, std::string> m;
   if(kv == "")
      return m;

   std::vector<std::string> parts;

   try
      {
      parts = split_on(kv, ',');
      }
   catch(std::exception&)
      {
      throw Invalid_Argument("Bad KV spec");
      }

   bool escaped = false;
   bool reading_key = true;
   std::string cur_key;
   std::string cur_val;

   for(char c : kv)
      {
      if(c == '\\' && !escaped)
         {
         escaped = true;
         }
      else if(c == ',' && !escaped)
         {
         if(cur_key.empty())
            throw Invalid_Argument("Bad KV spec empty key");

         if(m.find(cur_key) != m.end())
            throw Invalid_Argument("Bad KV spec duplicated key");
         m[cur_key] = cur_val;
         cur_key = "";
         cur_val = "";
         reading_key = true;
         }
      else if(c == '=' && !escaped)
         {
         if(reading_key == false)
            throw Invalid_Argument("Bad KV spec unexpected equals sign");
         reading_key = false;
         }
      else
         {
         if(reading_key)
            cur_key += c;
         else
            cur_val += c;

         if(escaped)
            escaped = false;
         }
      }

   if(!cur_key.empty())
      {
      if(reading_key == false)
         {
         if(m.find(cur_key) != m.end())
            throw Invalid_Argument("Bad KV spec duplicated key");
         m[cur_key] = cur_val;
         }
      else
         throw Invalid_Argument("Bad KV spec incomplete string");
      }

   return m;
   }

}
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void Timer::start()
   {
   stop();
   m_timer_start = OS::get_system_timestamp_ns();
   m_cpu_cycles_start = OS::get_cpu_cycle_counter();
   }

void Timer::stop()
   {
   if(m_timer_start)
      {
      if(m_cpu_cycles_start != 0)
         {
         const uint64_t cycles_taken = OS::get_cpu_cycle_counter() - m_cpu_cycles_start;
         if(cycles_taken > 0)
            {
            m_cpu_cycles_used += static_cast<size_t>(cycles_taken * m_clock_cycle_ratio);
            }
         }

      const uint64_t now = OS::get_system_timestamp_ns();

      if(now > m_timer_start)
         {
         const uint64_t dur = now - m_timer_start;

         m_time_used += dur;

         if(m_event_count == 0)
            {
            m_min_time = m_max_time = dur;
            }
         else
            {
            m_max_time = std::max(m_max_time, dur);
            m_min_time = std::min(m_min_time, dur);
            }
         }

      m_timer_start = 0;
      ++m_event_count;
      }
   }

bool Timer::operator<(const Timer& other) const
   {
   if(this->doing() != other.doing())
      return (this->doing() < other.doing());

   return (this->get_name() < other.get_name());
   }

std::string Timer::to_string() const
   {
   if(m_custom_msg.size() > 0)
      {
      return m_custom_msg;
      }
   else if(this->buf_size() == 0)
      {
      return result_string_ops();
      }
   else
      {
      return result_string_bps();
      }
   }

std::string Timer::result_string_bps() const
   {
   const size_t MiB = 1024 * 1024;

   const double MiB_total = static_cast<double>(events()) / MiB;
   const double MiB_per_sec = MiB_total / seconds();

   std::ostringstream oss;
   oss << get_name();

   if(!doing().empty())
      {
      oss << " " << doing();
      }

   if(buf_size() > 0)
      {
      oss << " buffer size " << buf_size() << " bytes:";
      }

   if(events() == 0)
      oss << " " << "N/A";
   else
      oss << " " << std::fixed << std::setprecision(3) << MiB_per_sec << " MiB/sec";

   if(cycles_consumed() != 0)
      {
      const double cycles_per_byte = static_cast<double>(cycles_consumed()) / events();
      oss << " " << std::fixed << std::setprecision(2) << cycles_per_byte << " cycles/byte";
      }

   oss << " (" << MiB_total << " MiB in " << milliseconds() << " ms)\n";

   return oss.str();
   }

std::string Timer::result_string_ops() const
   {
   std::ostringstream oss;

   oss << get_name() << " ";

   if(events() == 0)
      {
      oss << "no events\n";
      }
   else
      {
      oss << static_cast<uint64_t>(events_per_second())
          << ' ' << doing() << "/sec; "
          << std::setprecision(2) << std::fixed
          << ms_per_event() << " ms/op";

      if(cycles_consumed() != 0)
         {
         const double cycles_per_op = static_cast<double>(cycles_consumed()) / events();
         const size_t precision = (cycles_per_op < 10000) ? 2 : 0;
         oss << " " << std::fixed << std::setprecision(precision) << cycles_per_op << " cycles/op";
         }

      oss << " (" << events() << " " << (events() == 1 ? "op" : "ops")
          << " in " << milliseconds() << " ms)\n";
      }

   return oss.str();
   }

}
/*
* Version Information
* (C) 1999-2013,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
  These are intentionally compiled rather than inlined, so an
  application running against a shared library can test the true
  version they are running against.
*/

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

const char* short_version_cstr()
   {
   return STR(BOTAN_VERSION_MAJOR) "."
          STR(BOTAN_VERSION_MINOR) "."
          STR(BOTAN_VERSION_PATCH);
   }

const char* version_cstr()
   {

   /*
   It is intentional that this string is a compile-time constant;
   it makes it much easier to find in binaries.
   */

   return "Botan " STR(BOTAN_VERSION_MAJOR) "."
                   STR(BOTAN_VERSION_MINOR) "."
                   STR(BOTAN_VERSION_PATCH) " ("
#if defined(BOTAN_UNSAFE_FUZZER_MODE)
                   "UNSAFE FUZZER MODE BUILD "
#endif
                   BOTAN_VERSION_RELEASE_TYPE
#if (BOTAN_VERSION_DATESTAMP != 0)
                   ", dated " STR(BOTAN_VERSION_DATESTAMP)
#endif
                   ", revision " BOTAN_VERSION_VC_REVISION
                   ", distribution " BOTAN_DISTRIBUTION_INFO ")";
   }

#undef STR
#undef QUOTE

/*
* Return the version as a string
*/
std::string version_string()
   {
   return std::string(version_cstr());
   }

std::string short_version_string()
   {
   return std::string(short_version_cstr());
   }

uint32_t version_datestamp() { return BOTAN_VERSION_DATESTAMP; }

/*
* Return parts of the version as integers
*/
uint32_t version_major() { return BOTAN_VERSION_MAJOR; }
uint32_t version_minor() { return BOTAN_VERSION_MINOR; }
uint32_t version_patch() { return BOTAN_VERSION_PATCH; }

std::string runtime_version_check(uint32_t major,
                                  uint32_t minor,
                                  uint32_t patch)
   {
   if(major != version_major() || minor != version_minor() || patch != version_patch())
      {
      std::ostringstream oss;
      oss << "Warning: linked version (" << short_version_string() << ")"
          << " does not match version built against "
          << "(" << major << '.' << minor << '.' << patch << ")\n";
      return oss.str();
      }

   return "";
   }

}
