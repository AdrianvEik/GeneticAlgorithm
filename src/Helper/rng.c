
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>

#include "immintrin.h"
#include "intrin.h"

#include "rng.h"
#include "../Multiprocessing/mp_thread_locals.h"

#define UPPER_MASK		0x80000000
#define LOWER_MASK		0x7fffffff
#define TEMPERING_MASK_B	0x9d2c5680
#define TEMPERING_MASK_C	0xefc60000

struct mt_rand_s {
	uint32_t mt[STATE_VECTOR_LENGTH];
	int32_t index;
};

typedef struct mt_rand_s mt_rand_t;

// Thread local rng generator
thread_local mt_rand_t mt_thread;

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

	if (seed == NULL) {
		int bitseeker = 0x1;
		int bitcount = 0;
		while (bitcount < 5) {
			if (!rdrand32_retry(10, &seed)) {
				seed = random_int32();
			}
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


void seedRandThread(uint32_t seed) {
	mt_thread = seedRand(seed);
}

/**
 * Generates a pseudo-randomly generated long.
 */
uint32_t gen_mt_rand() {

	uint32_t y;
	static uint32_t mag[2] = { 0x0, 0x9908b0df }; /* mag[x] = x * 0x9908b0df for x = 0,1 */
	if (mt_thread.index >= STATE_VECTOR_LENGTH || mt_thread.index < 0) {
		/* generate STATE_VECTOR_LENGTH words at a time */
		int32_t kk;
		if (mt_thread.index >= STATE_VECTOR_LENGTH + 1 || mt_thread.index < 0) {
			m_seedRand(rand, 4357);
		}
		for (kk = 0; kk < STATE_VECTOR_LENGTH - STATE_VECTOR_M; kk++) {
			y = (mt_thread.mt[kk] & UPPER_MASK) | (mt_thread.mt[kk + 1] & LOWER_MASK);
			mt_thread.mt[kk] = mt_thread.mt[kk + STATE_VECTOR_M] ^ (y >> 1) ^ mag[y & 0x1];
		}
		for (; kk < STATE_VECTOR_LENGTH - 1; kk++) {
			y = (mt_thread.mt[kk] & UPPER_MASK) | (mt_thread.mt[kk + 1] & LOWER_MASK);
			mt_thread.mt[kk] = mt_thread.mt[kk + (STATE_VECTOR_M - STATE_VECTOR_LENGTH)] ^ (y >> 1) ^ mag[y & 0x1];
		}
		y = (mt_thread.mt[STATE_VECTOR_LENGTH - 1] & UPPER_MASK) | (mt_thread.mt[0] & LOWER_MASK);
		mt_thread.mt[STATE_VECTOR_LENGTH - 1] = mt_thread.mt[STATE_VECTOR_M - 1] ^ (y >> 1) ^ mag[y & 0x1];
		mt_thread.index = 0;
	}
	y = mt_thread.mt[mt_thread.index++];
	y ^= (y >> 11);
	y ^= (y << 7) & TEMPERING_MASK_B;
	y ^= (y << 15) & TEMPERING_MASK_C;
	y ^= (y >> 18);
	return y;
}
