

#include "rng.h"

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
__declspec(thread) mt_rand_t mt_thread;
__declspec(thread) sfmt_t sfmt_thread;

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

    if (rdrand_supported == -1) {
        rdrand_supported = is_RDRAND_supported();
    }

    if (rdrand_supported == 0) {
        return 0;
    }

	while (count <= retries) {
		if (_rdrand32_step(rand)) {
			return 1;
		}

		++count;
	}

	return 0;
}


unsigned int random_int32() {
	srand((unsigned int)time(0));
	return (rand() << 30) | (rand() << 15) | (rand());
}

//shared functions
uint32_t gen_mt_rand() {
    //return gen_mt_rand_internal();
    return sfmt_genrand_uint32(&sfmt_thread);
}

uint64_t gen_mt_rand64() {
	//return gen_mt_rand_internal();
	return sfmt_genrand_uint64(&sfmt_thread);
}

#if defined __AVX512VL__
__m512i gen_mt_rand512() {
	return sfmt_genrand_uint512(&sfmt_thread);
}
#endif // 

#if defined __AVX2__
__m256i gen_mt_rand256() {
	return sfmt_genrand_uint256(&sfmt_thread);
}
#endif // 

void seed_rand_threadlocal(uint32_t seed) {
    //seedRandThread_internal(seed);
	if (seed == 0) {
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
    sfmt_init_gen_rand(&sfmt_thread, seed);
}
