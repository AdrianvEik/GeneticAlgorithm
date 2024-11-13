
#ifndef _RNG_H
#define _RNG_H

#include <stdint.h>

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397



int rdrand();

void seedRandThread(uint32_t seed);
uint32_t gen_mt_rand();
int rdrand32_retry(unsigned int retries, uint32_t* rand);

//unsigned int random_int32();

#endif // _RNG_H