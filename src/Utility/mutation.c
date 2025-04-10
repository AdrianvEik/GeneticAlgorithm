#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include <immintrin.h>

#include "..\Helper\AVX_helper.h"
#include "mutation.h"
#include "..\Multiprocessing\mp_thread_locals.h"
#include "..\Helper\compile_flags.h"

inline unsigned int set_single_bit_switch(unsigned int position) {
	union int_bytes {
		int i;
		char c[4];
	};
	union int_bytes mask;
	mask.i = 0;
	switch (position & 0x1f8) {
	case 0:
		//first byte
		mask.c[3] = 1 << (position & 0x7);
		break;
	case 8:
		//third byte
		mask.c[2] = 1 << (position & 0x7);
		break;
	case 16:
		//second byte
		mask.c[1] = 1 << (position & 0x7);
		break;
	case 24:
		//first byte
		mask.c[0] = 1 << (position & 0x7);
		break;
	default:
		//unknown byte
		break;
	}
	return mask.i;
}

inline unsigned int set_single_bit_if(unsigned int position) {
	union int_bytes {
		unsigned int i;
		char c[4];
	};
	union int_bytes mask;
	mask.i = 0;
	int b = (position & 0x1f8);
	if (b == 0) mask.c[3] = 1 << (position & 0x7);
	if (b == 8) mask.c[2] = 1 << (position & 0x7);
	if (b == 16) mask.c[1] = 1 << (position & 0x7);
	if (b == 24) mask.c[0] = 1 << (position & 0x7);
	return mask.i;
}

inline unsigned int set_single_bit_divide(unsigned int position) {
	union int_bytes {
		int i;
		char c[4];
	};
	union int_bytes mask;
	mask.i = 0;
	int b = (position & 0x1f8) / 8;
	mask.c[b] = 1 << (position & 0x7); //it sets the wrong byte, but it does to consistently (it flips order of bytes)
	return mask.i;
}

inline unsigned int set_single_bit_divide_true(unsigned int position) {
	union int_bytes {
		int i;
		char c[4];
	};
	union int_bytes mask;
	mask.i = 0;
	int b = 3 - (position & 0x1f8) / 8;
	mask.c[b] = 1 << (position & 0x7);
	return mask.i;
}

inline unsigned int set_single_bit_shift_true(unsigned int position) {
	union int_bytes {
		int i;
		char c[4];
	};
	union int_bytes mask;
	mask.i = 0;
	int b = 3 - ((position & 0x1f8) >> 3);
	mask.c[b] = 1 << (position & 0x7);
	return mask.i;
}

#ifdef __AVX2__
inline __m256i set_single_bit_256_divide(unsigned int position) {
	union int256_bytes {
		__m256i i;
		char c[32];
	};
	union int256_bytes mask;
	mask.i = _mm256_setzero_si256();
	int b = (position & 0x1f8) / 8;
	mask.c[b] = 1 << (position & 0x7); //it sets the wrong byte, but it does to consistently (it flips order of bytes)
	return mask.i;
}

inline __m256i set_single_bit_256_divide_true(unsigned int position) {
	union int256_bytes {
		__m256i i;
		char c[32];
	};
	union int256_bytes mask;
	mask.i = _mm256_setzero_si256();
	int b = 31 - (position & 0x1f8) / 8;
	mask.c[b] = 1 << (position & 0x7); //it sets the wrong byte, but it does to consistently (it flips order of bytes)
	return mask.i;
}

inline __m256i set_single_bit_256_shift(unsigned int position) {
	union int256_bytes {
		__m256i i;
		char c[32];
	};
	union int256_bytes mask;
	mask.i = _mm256_setzero_si256();
	int b = (position & 0x1f8) >> 3; // divide by 8 == shift right 3
	mask.c[b] = 1 << (position & 0x7); //it sets the wrong byte, but it does to consistently (it flips order of bytes)
	return mask.i;
}

inline __m256i set_single_bit_256_shift_true(unsigned int position) {
	union int256_bytes {
		__m256i i;
		char c[32];
	};
	union int256_bytes mask;
	mask.i = _mm256_setzero_si256();
	int b = 31 - ((position & 0x1f8) >> 3); // divide by 8 == shift right 3
	mask.c[b] = 1 << (position & 0x7); //it sets the wrong byte, but it does to consistently (it flips order of bytes)
	return mask.i;
}
#endif

#ifdef __AVX512VL__
inline __m512i set_single_bit_512_shift(uint32_t position) {
	union int512_bytes {
		__m512i i;
		char c[64];
	};
	union int512_bytes mask;
	mask.i = _mm512_setzero_si512();
	int b = (position & 0x1f8) >> 3; // divide by 8 == shift right 3
	mask.c[b] = 1 << (position & 0x7); //it sets the wrong byte, but it does to consistently (it flips order of bytes)
	return mask.i;
}

inline __m512i set_single_bit_512_shift_true(uint32_t position) {
	union int512_bytes {
		__m512i i;
		char c[64];
	};
	union int512_bytes mask;
	mask.i = _mm512_setzero_si512();
	int b = 63 - ((position & 0x1f8) >> 3); // divide by 8 == shift right 3
	mask.c[b] = 1 << (position & 0x7); //it sets the wrong byte, but it does to consistently (it flips order of bytes)
	return mask.i;
}
#endif

void mutate32(gene_pool_t* gene_pool, mutation_param_t* mutation_param) {
	exit(1);
	/*

	This function mutates a bitarray by flipping a random bit.

	:param bit: bitarray to mutate
	:type bit: int*

	:param size: size of the bitarray
	:type size: int

	:param mutate_coeff_rate: amount of mutations over the bitarray
	:type mutate_coeff_rate: int

	:param chaos_coeff: the signifigance of the bits impacted by the mutation (1 to 32) (1 for least significant bit, 32 for most significant bit)
	:type chaos_coeff: int

	:param allow_sign_flip: whether or not to allow the sign to flip, 1 for yes, 0 for no
	:type allow_sign_flip: int

	*/

	// int* mutations = malloc(gene_pool->genes * sizeof(int));
	// // generate random mutations, that are not at the same position
	// for (int i = 0; i < gene_pool->genes; i++){
	//     mutations[i] = 0;
	// }

	uint32_t mutation_rnd;
	int mutation_gene; 
	int mutation_bit_mask = 0;
	for (int i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
		for (int j = 0; j < mutation_param->mutation_rate[i]; j++) { // check if works
			// ensure that the selected gene is positive
            mutation_rnd = gen_mt_rand();
			int bit_pos = ((mutation_rnd & 0xf8000000) / 0x08000000u);
			mutation_bit_mask = (int)1 << bit_pos; // mask, 5 bits describe 32 positions
			mutation_gene = (mutation_rnd & 0x7ffffff) % gene_pool->genes;
			gene_pool->pop_param_bin[gene_pool->sorted_indexes[i]][mutation_gene] ^= mutation_bit_mask;
		}
	}
}

void mutateAVX(gene_pool_t* gene_pool, mutation_param_t* mutation_param) {

	/*

	This function mutates a bitarray by flipping a random bit.

	:param bit: bitarray to mutate
	:type bit: int*

	:param size: size of the bitarray
	:type size: int

	:param mutate_coeff_rate: amount of mutations over the bitarray
	:type mutate_coeff_rate: int

	:param chaos_coeff: the signifigance of the bits impacted by the mutation (1 to 32) (1 for least significant bit, 32 for most significant bit)
	:type chaos_coeff: int

	:param allow_sign_flip: whether or not to allow the sign to flip, 1 for yes, 0 for no
	:type allow_sign_flip: int

	*/

	// int* mutations = malloc(gene_pool->genes * sizeof(int));
	// // generate random mutations, that are not at the same position
	// for (int i = 0; i < gene_pool->genes; i++){
	//     mutations[i] = 0;
	// }

	uint32_t mutation_rnd;
	uint32_t mutation_gene_AVX;

#ifdef __AVX512VL__
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__m512i);
	__m512i** pop_param_bin_ptr = (__m512i**)gene_pool->pop_param_bin;
	union int512_bytes {
		__m512i i;
		char c[64];
	};
	union int512_bytes mask;
#else 
	#ifdef __AVX2__
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__m256i);
	__m256i** pop_param_bin_ptr = (__m256i**)gene_pool->pop_param_bin;
	union int256_bytes {
		__m256i i;
		char c[32];
	};
	union int256_bytes mask;
#else
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(uint32_t);
	uint32_t** pop_param_bin_ptr = (uint32_t**)gene_pool->pop_param_bin;
	union int_bytes {
		int i;
		char c[4];
	};
	union int_bytes mask;
#endif
#endif


	for (int i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
		for (int j = 0; j < mutation_param->mutation_rate[i]; j++) { // check if works
			// ensure that the selected gene is positive
			mutation_rnd = gen_mt_rand();
			//mutation_bit_mask = set_single_bit_512_devide_true((mutation_rnd & 0xff800000u) / 0x00800000u); //choose bits for location, 9 bits describe 512 positions
#ifdef __AVX512VL__
			mutation_gene_AVX = (mutation_rnd >> 9) % memory_blocks;
			mask.i = _mm512_setzero_si512();
			int b = (mutation_rnd & 0x1f8) >> 3; // divide by 8 == shift right 3, it sets the wrong byte, but it does to consistently (it flips order of bytes)
			mask.c[b] = 1 << (mutation_rnd & 0x7);
			pop_param_bin_ptr[gene_pool->sorted_indexes[i]][mutation_gene_AVX] = _mm512_xor_si512(
				pop_param_bin_ptr[gene_pool->sorted_indexes[i]][mutation_gene_AVX],
				mask.i
			);
#else 
#ifdef __AVX2__
			mutation_gene_AVX = (mutation_rnd >> 8) % memory_blocks;
			mask.i = _mm256_setzero_si256();
			int b = (mutation_rnd & 0xf8) >> 3; // divide by 8 == shift right 3, it sets the wrong byte, but it does to consistently (it flips order of bytes)
			mask.c[b] = 1 << (mutation_rnd & 0x7);
			pop_param_bin_ptr[gene_pool->sorted_indexes[i]][mutation_gene_AVX] = _mm256_xor_si256(
				pop_param_bin_ptr[gene_pool->sorted_indexes[i]][mutation_gene_AVX],
				mask.i
			);
#else
			mutation_gene_AVX = (mutation_rnd >> 5) % memory_blocks;
			mask.i = 0;
			int b = (mutation_rnd & 0x18) >> 3;
			mask.c[b] = 1 << (mutation_rnd & 0x7);
			pop_param_bin_ptr[gene_pool->sorted_indexes[i]][mutation_gene_AVX] =
				pop_param_bin_ptr[gene_pool->sorted_indexes[i]][mutation_gene_AVX] ^
				mask.i
			;
#endif
#endif
		}
	}
}

void mutateAVXFast(gene_pool_t* gene_pool, mutation_param_t* mutation_param) {

	/*

	This function mutates a bitarray by flipping a random bit.

	:param bit: bitarray to mutate
	:type bit: int*

	:param size: size of the bitarray
	:type size: int

	:param mutate_coeff_rate: amount of mutations over the bitarray
	:type mutate_coeff_rate: int

	:param chaos_coeff: the signifigance of the bits impacted by the mutation (1 to 32) (1 for least significant bit, 32 for most significant bit)
	:type chaos_coeff: int

	:param allow_sign_flip: whether or not to allow the sign to flip, 1 for yes, 0 for no
	:type allow_sign_flip: int

	*/

	// int* mutations = malloc(gene_pool->genes * sizeof(int));
	// // generate random mutations, that are not at the same position
	// for (int i = 0; i < gene_pool->genes; i++){
	//     mutations[i] = 0;
	// }

	uint32_t mutation_rnd;
	//uint32_t* mutation_per_memoryblock;

	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__mAVXi);
	__mAVXi** pop_param_bin_ptr = (__mAVXi**)gene_pool->pop_param_bin;
	union AVX_union_bytes {
		__mAVXi i;
		uint32_t c[AVX_dwords];
	};
	union AVX_union_bytes mask;
	mask.i = AVX_setzero();

	//mutation_per_memoryblock = calloc(memory_blocks, sizeof(uint32_t));
	double mutation_per_memoryblock;
	mutation_rnd = gen_mt_rand();
	//uint32_t memblock_rng_bits_left = 32;
	double mutation_factor = (2.0 / (double)memory_blocks) / (double)INT32_MAX;

	for (int i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
		//for (int j = 0; j < mutation_param->mutation_rate[i]; j++) {
		//	mutation_per_memoryblock[gen_mt_rand() % memory_blocks]++;
		//}
		for (uint32_t memory_block = 0; memory_block < memory_blocks; memory_block++) {
			mutation_per_memoryblock = mutation_param->mutation_rate[i] * (double)gen_mt_rand() * mutation_factor;
			if (mutation_per_memoryblock > 1) {
				mask.i = AVX_setzero();
				for (double k = 0.0; k < mutation_per_memoryblock; k++) {
					//if (memblock_rng_bits_left < AVX_dwordpointer_bits * 3) {
					mutation_rnd = gen_mt_rand();
					//	memblock_rng_bits_left = 32;
					//}
					uint32_t b = (mutation_rnd >> 3) & AVX_dwordpointer_mask; // divide by 8 == shift right 3, it sets the wrong byte, but it does to consistently (it flips order of bytes)
					mask.c[b] |= 1 << (mutation_rnd & 0x1f);
					mutation_rnd = mutation_rnd >> AVX_bitpointer_bits;
					//if (mutation_per_memoryblock - k > 1.5f) {
					//	b = (mutation_rnd >> 3) & AVX_dwordpointer_mask; // divide by 8 == shift right 3, it sets the wrong byte, but it does to consistently (it flips order of bytes)
					//	mask.c[b] &= 1 << (mutation_rnd & 0x1f);
					//	mutation_rnd = mutation_rnd >> AVX_bitpointer_bits;
						//b = (mutation_rnd >> 3) & AVX_dwordpointer_mask; // divide by 8 == shift right 3, it sets the wrong byte, but it does to consistently (it flips order of bytes)
						//mask.c[b] &= 1 << (mutation_rnd & 0x1f);
						//mutation_rnd = mutation_rnd >> AVX_bitpointer_bits;
					/*}*/
      //              else if (mutation_per_memoryblock - k > 1.5f) {
						//b = (mutation_rnd >> 3) & AVX_dwordpointer_mask; // divide by 8 == shift right 3, it sets the wrong byte, but it does to consistently (it flips order of bytes)
						//mask.c[b] &= 1 << (mutation_rnd & 0x1f);
						//mutation_rnd = mutation_rnd >> AVX_bitpointer_bits;
      //              }
				}
				pop_param_bin_ptr[gene_pool->sorted_indexes[i]][memory_block] = AVX_xor(
					pop_param_bin_ptr[gene_pool->sorted_indexes[i]][memory_block],
					mask.i
				);
				//mutation_per_memoryblock[memory_block] = 0; // reset the needed mutations, otherwise it cumulates...
			}
		}
	}
}

void process_mutation(gene_pool_t* gene_pool, mutation_param_t* mutation_param) {
    /*
    */
    // Check if the distributions are up to date
	mutateAVXFast(gene_pool, mutation_param);
}