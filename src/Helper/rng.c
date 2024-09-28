
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

#include "immintrin.h"
#include "intrin.h"

#include "rng.h"

#define UPPER_MASK		0x80000000
#define LOWER_MASK		0x7fffffff
#define TEMPERING_MASK_B	0x9d2c5680
#define TEMPERING_MASK_C	0xefc60000

int rdrand_supported = -1;

int is_RDRAND_supported()
{
	int name[4] = { 0 };
	__cpuid(name, 0);

	if (name[1] == 0x756e6547 &&         //uneG
		name[2] == 0x6c65746e &&        //letn
		name[3] == 0x49656e69)          //Ieni
	{
		int data[4] = { 0 };
		__cpuid(data, 1);

		//Check bit 30 on the 2nd index (ECX register)
		if (data[2] & (0x1 << 30))
		{
			//Supported!
			return 1;
		}
	}

	return 0;
}

int rdrand32_retry(unsigned int retries, uint32_t* rand)
{
	unsigned int count = 0;

	while (count <= retries) {
		if (_rdrand32_step(rand)) {
			return 1;
		}

		++count;
	}

	return 0;
}

int rdrand() {
	uint32_t rand_res;
	if (rdrand_supported == -1) {
		rdrand_supported = is_RDRAND_supported();
	}
	if (rdrand_supported == 1) {
		if (rdrand32_retry(10, &rand_res)) {
			return rand_res;
		}
		else {
			printf("RDRAND instruction failed\n");
			exit(250);
		}
	}
	else if ((rdrand_supported == 1)) {
		srand((unsigned int)time(0));
		return (rand() << 30) | (rand() << 15) | (rand());
	}

	//}
	//else
	//{
	//	srand((unsigned int)time(0));
	//	return (rand() << 30) | (rand() << 15) | (rand());
	//}

 //   printf("RDRAND instruction not supported\n");
	//exit(250);
}


inline static void m_seedRand(mt_rand_t* rand, uint32_t seed) {
	/* set initial seeds to mt[STATE_VECTOR_LENGTH] using the generator
	 * from Line 25 of Table 1 in: Donald Knuth, "The Art of Computer
	 * Programming," Vol. 2 (2nd Ed.) pp.102.
	 */
	rand->mt[0] = seed & 0xffffffff;
	for (rand->index = 1; rand->index < STATE_VECTOR_LENGTH; rand->index++) {
		rand->mt[rand->index] = (6069 * rand->mt[rand->index - 1]) & 0xffffffff;
	}
}

unsigned int random_int32() {
	srand((unsigned int)time(0));
	return (rand() << 30) | (rand() << 15) | (rand());
}

/**
* Creates a new random number generator from a given seed.
*/
mt_rand_t seedRand(uint32_t seed) {
	mt_rand_t rand;

	if (seed != NULL) {
		int bitseeker = 0x1;
		int bitcount = 0;
		while (bitcount < 5) {
			seed = rdrand();
			for (int i = 0; i < 32; i++) {
				if (seed & (bitseeker << i)) {
					bitcount++;
				}
			}
		}
	}
	m_seedRand(&rand, seed);

	return rand;
}

void init_thread_rng(int thread_cnt) {
	thread_count = thread_cnt;
    mt_rand_lookup = (mt_rand_t*)malloc(sizeof(mt_rand_t) * thread_count);
    mt_rand_thread_mapper = (int*)malloc(sizeof(int) * thread_count);
}

void free_thread_rng() {
    free(mt_rand_lookup);
    free(mt_rand_thread_mapper);
}

void seedRandThread(uint32_t thread_id, uint32_t seed) {
	mt_rand_thread_mapper[thread_id] = pthread_getw32threadid_np(pthread_self());
	mt_rand_lookup[thread_id] = seedRand(seed);
}

mt_rand_t *getThreadRand() {
    int thread_id = pthread_getw32threadid_np(pthread_self());
    for (int i = 0; i < thread_count; i++) {
        if (mt_rand_thread_mapper[i] == thread_id) {
            return &mt_rand_lookup[i];
        }
    }
	printf("Thread not found\n");
	exit(255);
}

/**
 * Generates a pseudo-randomly generated long.
 */
uint32_t gen_mt_rand(mt_rand_t* rand) {

	uint32_t y;
	static uint32_t mag[2] = { 0x0, 0x9908b0df }; /* mag[x] = x * 0x9908b0df for x = 0,1 */
	if (rand->index >= STATE_VECTOR_LENGTH || rand->index < 0) {
		/* generate STATE_VECTOR_LENGTH words at a time */
		int32_t kk;
		if (rand->index >= STATE_VECTOR_LENGTH + 1 || rand->index < 0) {
			m_seedRand(rand, 4357);
		}
		for (kk = 0; kk < STATE_VECTOR_LENGTH - STATE_VECTOR_M; kk++) {
			y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
			rand->mt[kk] = rand->mt[kk + STATE_VECTOR_M] ^ (y >> 1) ^ mag[y & 0x1];
		}
		for (; kk < STATE_VECTOR_LENGTH - 1; kk++) {
			y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
			rand->mt[kk] = rand->mt[kk + (STATE_VECTOR_M - STATE_VECTOR_LENGTH)] ^ (y >> 1) ^ mag[y & 0x1];
		}
		y = (rand->mt[STATE_VECTOR_LENGTH - 1] & UPPER_MASK) | (rand->mt[0] & LOWER_MASK);
		rand->mt[STATE_VECTOR_LENGTH - 1] = rand->mt[STATE_VECTOR_M - 1] ^ (y >> 1) ^ mag[y & 0x1];
		rand->index = 0;
	}
	y = rand->mt[rand->index++];
	y ^= (y >> 11);
	y ^= (y << 7) & TEMPERING_MASK_B;
	y ^= (y << 15) & TEMPERING_MASK_C;
	y ^= (y >> 18);
	return y;
}
