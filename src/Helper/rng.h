
#ifndef _RNG_H
#define _RNG_H

#include <stdint.h>

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397

struct mt_rand_s {
    uint32_t mt[STATE_VECTOR_LENGTH];
    int32_t index;
};

typedef struct mt_rand_s mt_rand_t;

mt_rand_t* mt_rand_lookup;
int* mt_rand_thread_mapper;

int rdrand();
void init_thread_rng(int thread_count);
void free_thread_rng();

void seedRandThread(uint32_t thread_id, uint32_t seed);
mt_rand_t seedRand(uint32_t seed);
mt_rand_t *getThreadRand();
unsigned int random_int32();
uint32_t gen_mt_rand(mt_rand_t* rand);

#endif // _RNG_H