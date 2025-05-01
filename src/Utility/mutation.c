
#include "mutation.h"

void process_mutation(gene_pool_t* gene_pool, mutation_param_t* mutation_param) {

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

				}
				pop_param_bin_ptr[gene_pool->sorted_indexes[i]][memory_block] = AVX_xor(
					pop_param_bin_ptr[gene_pool->sorted_indexes[i]][memory_block],
					mask.i
				);
			}
		}
	}
}
