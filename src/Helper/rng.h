
#ifndef _RNG_H
#define _RNG_H

#include <stdint.h>
#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <intrin.h>

#include "../Multiprocessing/mp_thread_locals.h"
#include "../Multiprocessing/mp_thread_wrapper.h"

#include "rng/SFMT.h"

#include "compile_flags.h"

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397

//int rdrand();

/**
 * Seed the thread-local SFMT random generator.
 *
 * Passing ``0`` requests an entropy seed from RDRAND when available, falling
 * back to the C runtime RNG.
 *
 * :param seed: Explicit seed value, or ``0`` to auto-seed.
 */
void seed_rand_threadlocal(uint32_t seed);

/**
 * Generate a 32-bit unsigned random integer from the thread-local SFMT state.
 *
 * :returns: Random ``uint32_t`` value.
 */
uint32_t gen_mt_rand();

/**
 * Generate a 64-bit unsigned random integer from the thread-local SFMT state.
 *
 * :returns: Random ``uint64_t`` value.
 */
uint64_t gen_mt_rand64();

#ifdef __AVX512VL__
/**
 * Generate a 512-bit random vector when AVX-512 support is enabled.
 *
 * :returns: Random ``__m512i`` vector.
 */
__m512i gen_mt_rand512();
#endif

#ifdef __AVX2__
/**
 * Generate a 256-bit random vector when AVX2 support is enabled.
 *
 * :returns: Random ``__m256i`` vector.
 */
__m256i gen_mt_rand256();
#endif


//int rdrand32_retry(unsigned int retries, uint32_t* rand);

//unsigned int random_int32();

#endif // _RNG_H
