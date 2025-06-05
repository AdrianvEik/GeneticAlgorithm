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
#define AVX_bits 512
#define AVX_bitpointer_bits 9
#define AVX_bitpointer_mask 0x1ff
//#define AVX_bytepointer_bits 6
//#define AVX_bytepointer_mask 0x3f
#define AVX_dwordpointer_bits 4
#define AVX_dwordpointer_mask 0xf
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
#define AVX_bits 256
#define AVX_bitpointer_bits 8
#define AVX_bitpointer_mask 0xff
//#define AVX_bytepointer_bits 5
//#define AVX_bytepointer_mask 0x1f
#define AVX_dwordpointer_bits 3
#define AVX_dwordpointer_mask 0x7
#else
#define __mAVXi uint64_t
#define AVX_setzero() 0
#define AVX_setone() 0xffffffffffffffff
#define AVX_and(left, right)      left & right
#define AVX_or(left, right)       left | right
#define AVX_xor(left, right)      left ^ right
//#define AVX_bytes 8
#define AVX_dwords 2
#define AVX_bits 64
#define AVX_bitpointer_bits 6
#define AVX_bitpointer_mask 0x3f
//#define AVX_bytepointer_bits 2
//#define AVX_bytepointer_mask 0x3
#define AVX_dwordpointer_bits 1
#define AVX_dwordpointer_mask 0x1
#endif
#endif

#endif // AVX_HELPER_H