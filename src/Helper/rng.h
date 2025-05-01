
#ifndef _RNG_H
#define _RNG_H

#include <stdint.h>
#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <intrin.h>

#include "../Multiprocessing/mp_thread_locals.h"

#include "../helper/rng/SFMT.h"

#include "compile_flags.h"

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397

//int rdrand();

void seedRandThread(uint32_t seed);
uint32_t gen_mt_rand();
uint64_t gen_mt_rand64();

#ifdef __AVX512VL__
__m512i gen_mt_rand512();
#endif

#ifdef __AVX2__
__m256i gen_mt_rand256();
#endif


//int rdrand32_retry(unsigned int retries, uint32_t* rand);

//unsigned int random_int32();

#endif // _RNG_H