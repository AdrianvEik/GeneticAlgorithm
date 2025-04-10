#ifndef AVX_HELPER_H
#define AVX_HELPER_H

#include <stdint.h>
#include "compile_flags.h"

#ifdef __AVX512VL__
#define __mAVXi __m512i
#define AVX_setzero()   _mm512_setzero_si512()
#define AVX_setone()    _mm512_set1_epi32(-1)
#define AVX_and(left, right)                 _mm512_and_epi32(left, right)
#define AVX_or(left, right)                  _mm512_or_epi32(left, right)
#define AVX_xor(left, right)                 _mm512_xor_epi32(left, right)
//#define AVX_bytes 64
#define AVX_dwords 16
static const uint32_t AVX_bits = 512;
static const uint32_t AVX_bitpointer_bits = 9;
static const uint32_t AVX_bitpointer_mask = 0x1ff;
//static const uint32_t AVX_bytepointer_bits = 6;
//static const uint32_t AVX_bytepointer_mask = 0x3f;
static const uint32_t AVX_dwordpointer_bits = 4;
static const uint32_t AVX_dwordpointer_mask = 0xf;
#else
#ifdef __AVX2__
#define __mAVXi __m256i
#define AVX_setzero() _mm256_setzero_si256()
#define AVX_setone() _mm256_set1_epi32(-1)
#define AVX_and(left, right)                 _mm256_and_epi32(left, right)
#define AVX_or(left, right)                  _mm256_or_epi32(left, right)
#define AVX_xor(left, right)                 _mm256_xor_epi32(left, right)
//#define AVX_bytes 32
#define AVX_dwords 8
static const uint32_t AVX_bits = 256;
static const uint32_t AVX_bitpointer_bits = 8;
static const uint32_t AVX_bitpointer_mask = 0xff;
//static const uint32_t AVX_bytepointer_bits = 5;
//static const uint32_t AVX_bytepointer_mask = 0x1f;
static const uint32_t AVX_dwordpointer_bits = 3;
static const uint32_t AVX_dwordpointer_mask = 0x7;
#else
#define __mAVXi uint64_t
#define AVX_setzero() 0
#define AVX_setone() 0xffffffffffffffff
#define AVX_and(left, right)      left & right
#define AVX_or(left, right)       left | right
#define AVX_xor(left, right)      left ^ right
//#define AVX_bytes 8
#define AVX_dwords 2
static const uint32_t AVX_bits = 64;
static const uint32_t AVX_bitpointer_bits = 6;
static const uint32_t AVX_bitpointer_mask = 0x3f;
//static const uint32_t AVX_bytepointer_bits = 2;
//static const uint32_t AVX_bytepointer_mask = 0x3;
static const uint32_t AVX_dwordpointer_bits = 1;
static const uint32_t AVX_dwordpointer_mask = 0x1;
#endif
#endif

#endif // AVX_HELPER_H