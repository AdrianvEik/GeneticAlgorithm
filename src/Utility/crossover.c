#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include <immintrin.h>
#include <string.h>

#include "crossover.h"
#include "..\Helper\AVX_helper.h"




// Path: Utility/crossover.c

static void single_point_crossoverAVX(int* parent1, int* parent2, int* child1, int* child2, int genes, int individual_mem_size) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// The function should fill child1 and child2 with the crossed over values


//#ifdef __AVX512VL__
	uint32_t memory_blocks = individual_mem_size / sizeof(__mAVXi);

	__mAVXi* parent1_ptr = (__mAVXi*)parent1;
	__mAVXi* parent2_ptr = (__mAVXi*)parent2;
	__mAVXi* child1_ptr = (__mAVXi*)child1;
	__mAVXi* child2_ptr = (__mAVXi*)child2;

	union AVX_union_bytes {
		__mAVXi i;
		uint32_t c[AVX_dwords];
	};
	union AVX_union_bytes mask;
	uint32_t crosspoint_rnd = gen_mt_rand();
	uint32_t crosspoint_gene_AVX = (crosspoint_rnd >> AVX_bitpointer_bits) % memory_blocks;
	mask.i = AVX_setzero();
	uint32_t b = ((crosspoint_rnd >> 5) & AVX_dwordpointer_mask);
#ifdef __AVX512VL__
	uint16_t set_mask = (uint16_t)0xffffu << (AVX_dwords - b);
    _mm512_mask_set1_epi32(mask.i, set_mask, 0xffffffff);
#else 
#ifdef __AVX2__
	uint8_t set_mask = (uint8_t)0xffu << b;
	_mm256_mask_set1_epi32(mask.i, set_mask, 0xffffffff);
#else
	for (uint32_t j = 0; j < b; j++) {
		mask.c[j] = 0xffffffff;
	}
#endif
#endif
	mask.c[b] = 0xffffffff << (crosspoint_rnd & 0x1f);

	for (uint32_t i = 0; i < memory_blocks; i++) {
		if (i < crosspoint_gene_AVX) {
			child1_ptr[i] = parent1_ptr[i];
			child2_ptr[i] = parent2_ptr[i];
		}
		else if (i > crosspoint_gene_AVX) {
			child1_ptr[i] = parent2_ptr[i];
			child2_ptr[i] = parent1_ptr[i];
		}
		else {
			child1_ptr[i] = AVX_or(
				AVX_and(
					parent1_ptr[i],
					mask.i),
				AVX_and(
					parent2_ptr[i],
					AVX_xor(mask.i, AVX_setone())
				)
			);
			child2_ptr[i] = AVX_or(
				AVX_and(
					parent1_ptr[i],
					AVX_xor(mask.i, AVX_setone())
				),
				AVX_and(
					parent2_ptr[i],
					mask.i
				)
			);
		}
	}
}


// Actually PMX crossover
static void two_point_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// point1 and point2 are the points to cross over at
	// The function should fill child1 and child2 with the crossed over values

	// find two random points to cross over

	int point1 = gen_mt_rand() % genes * sizeof(int) * 8 - 3;
	int point2 = (gen_mt_rand() % (genes * sizeof(int) * 8 - 2 - point1)) + point1 + 1;

	// make sure point1 is less than point2 and less than size

	int mask;
	int bit_i;
	for (int i = 0; i < genes; i++) {
		bit_i = i * sizeof(int) * 8;
		if (bit_i < point1 - sizeof(int) * 8) {
			mask = 0x0;
		}
		else if (bit_i < point1 && bit_i > point2 - sizeof(int) * 8) {
			mask = ((1 << (i - point1)) - 1) && ~((1 << (i - point2)) - 1);
		}
		else if (bit_i < point1) {
			mask = (1 << (i - point1)) - 1;
		}
		else if (bit_i < point2 - sizeof(int) * 8) {
			mask = 0xffffffff;
		}
		else if (bit_i < point2) {
			mask = ~((1 << (i - point2)) - 1);
		}
		else {
			mask = 0x0;
		}

		child1[i] = (parent1[i] & ~mask) | (parent2[i] & mask);
		child2[i] = (parent1[i] & mask) | (parent2[i] & ~mask);
	}
}

static void two_point_crossoverAVX(int* parent1, int* parent2, int* child1, int* child2, int genes, int individual_mem_size) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// The function should fill child1 and child2 with the crossed over values


//#ifdef __AVX512VL__
	uint32_t memory_blocks = individual_mem_size / sizeof(__mAVXi);

	__mAVXi* parent1_ptr = (__mAVXi*)parent1;
	__mAVXi* parent2_ptr = (__mAVXi*)parent2;
	__mAVXi* child1_ptr = (__mAVXi*)child1;
	__mAVXi* child2_ptr = (__mAVXi*)child2;

	uint32_t crosspoint_rnd_a = gen_mt_rand() % (individual_mem_size * 8);
	uint32_t crosspoint_rnd_b = gen_mt_rand() % (individual_mem_size * 8);

	while (crosspoint_rnd_a == crosspoint_rnd_b) {
        crosspoint_rnd_b = gen_mt_rand() % (individual_mem_size * 8);
	}
	
	//if (crosspoint_rnd_a == crosspoint_rnd_b) crosspoint_rnd_b++;

    if (crosspoint_rnd_b < crosspoint_rnd_a) {
        uint32_t temp = crosspoint_rnd_a;
        crosspoint_rnd_a = crosspoint_rnd_b;
        crosspoint_rnd_b = temp;
    }

	union AVX_union_bytes {
		__mAVXi i;
		uint32_t c[AVX_dwords];
	};
	union AVX_union_bytes mask_a;

	uint32_t crosspoint_memblock_a_AVX = (crosspoint_rnd_a >> AVX_bitpointer_bits);
	mask_a.i = AVX_setzero();
	uint32_t crossbyte_a = (crosspoint_rnd_a >> 5) & AVX_dwordpointer_mask;

	union AVX_union_bytes mask_b;
	uint32_t crosspoint_memblock_b_AVX = (crosspoint_rnd_b >> AVX_bitpointer_bits);
	mask_b.i = AVX_setzero();
	uint32_t crossbyte_b = (crosspoint_rnd_b >> 5) & AVX_dwordpointer_mask;
#ifdef __AVX512VL__
	uint16_t set_mask_a = (uint16_t)0xffffu >> (AVX_dwords - crossbyte_a);
	mask_a.i = _mm512_mask_set1_epi32(mask_a.i, set_mask_a, 0xffffffff);
	uint16_t set_mask_b = (uint16_t)0xffffu << (crossbyte_b);
	mask_b.i = _mm512_mask_set1_epi32(mask_b.i, set_mask_b, 0xffffffff);
#else 
#ifdef __AVX2__
	uint8_t set_mask_a = (uint8_t)0xffu << crossbyte_a;
	mask_a.i = _mm256_mask_set1_epi32(mask_a.i, set_mask_a, 0xffffffff);

	uint8_t set_mask_b = (uint8_t)0xffu >> crossbyte_b;
	mask_b.i = _mm256_mask_set1_epi32(mask_b.i, set_mask_b, 0xffffffff);
#else
	for (uint32_t j = 0; j < crossbyte_a; j++) {
		mask_a.c[j] = 0xffffffff;
	}

	for (uint32_t j = crossbyte_b + 1; j < 3; j++) {
		mask_b.c[j] = 0xffffffff;
	}
#endif
#endif
	mask_a.c[crossbyte_a] = 0xffffffff << (32 - (crosspoint_rnd_a & 0x1f));
	mask_b.c[crossbyte_b] = 0xffffffff >> (crosspoint_rnd_b & 0x1f);

	for (uint32_t i = 0; i < memory_blocks; i++) {
		if (i < crosspoint_memblock_a_AVX || i > crosspoint_memblock_b_AVX) {
			child1_ptr[i] = parent1_ptr[i];
			child2_ptr[i] = parent2_ptr[i];
		}
		else if (i > crosspoint_memblock_a_AVX || i < crosspoint_memblock_b_AVX) {
			child1_ptr[i] = parent2_ptr[i];
			child2_ptr[i] = parent1_ptr[i];
		}
		else if (i == crosspoint_memblock_a_AVX && i == crosspoint_memblock_b_AVX){
			child1_ptr[i] = AVX_or(
				AVX_and(
					parent1_ptr[i],
					AVX_or(
						mask_a.i,
						mask_b.i
					)
				),
				AVX_and(
					parent2_ptr[i],
					AVX_xor(AVX_or(
						mask_a.i,
						mask_b.i
					), AVX_setone()
					)
				)
			);
			child2_ptr[i] = AVX_or(
				AVX_and(
					parent1_ptr[i],
					AVX_xor(AVX_or(
						mask_a.i,
						mask_b.i
					), AVX_setone()
					)
                ),
				AVX_and(
					parent2_ptr[i],
					AVX_or(
						mask_a.i,
						mask_b.i
					)
				)
			);
		}
		else if (i == crosspoint_memblock_a_AVX) {
			child1_ptr[i] = AVX_or(
				AVX_and(
					parent1_ptr[i],
					mask_a.i	
				),
				AVX_and(
					parent2_ptr[i],
                    AVX_xor(mask_a.i, AVX_setone())
				)
			);
			child2_ptr[i] = AVX_or(
				AVX_and(
					parent1_ptr[i],
					AVX_xor(mask_a.i, AVX_setone())
				),
				AVX_and(
					parent2_ptr[i],
					mask_a.i
				)
			);
		}
        else { // i == crosspoint_memblock_b_AVX
            child1_ptr[i] = AVX_or(
                AVX_and(
                    parent1_ptr[i],
                    mask_b.i
                ),
                AVX_and(
                    parent2_ptr[i],
					AVX_xor(mask_b.i, AVX_setone())
                )
            );
            child2_ptr[i] = AVX_or(
                AVX_and(
                    parent1_ptr[i],
					AVX_xor(mask_b.i, AVX_setone())
				),
                AVX_and(
                    parent2_ptr[i],
                    mask_b.i
                )
            );
		}
	}
}

static void uniform_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// prob is the probability of a value being copied from the first parent
	// The function should fill child1 and child2 with the crossed over values

	// int mask = pow(2, point) - 1;

	int mask;

	for (int i = 0; i < genes; i++) {

		mask = gen_mt_rand();

		child1[i] = (parent1[i] & ~mask) | (parent2[i] & mask);
		child2[i] = (parent1[i] & mask) | (parent2[i] & ~mask);
	}
}

static void uniform_crossoverAVX(int* parent1, int* parent2, int* child1, int* child2, int genes, int individual_mem_size) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// prob is the probability of a value being copied from the first parent
	// The function should fill child1 and child2 with the crossed over values

	// int mask = pow(2, point) - 1;

#ifdef __AVX512VL__
	__m512i mask;
	uint32_t memory_blocks = individual_mem_size / sizeof(__m512i);

	__m512i* parent1_ptr = (__m512i*)parent1;
	__m512i* parent2_ptr = (__m512i*)parent2;
	__m512i* child1_ptr = (__m512i*)child1;
	__m512i* child2_ptr = (__m512i*)child2;

	for (uint32_t i = 0; i < memory_blocks; i++) {
		mask = gen_mt_rand512();

		child1_ptr[i] = _mm512_or_epi64((_mm512_andnot_epi64(parent1_ptr[i], mask)), (_mm512_and_epi64(parent2_ptr[i], mask)));
		child2_ptr[i] = _mm512_or_epi64((_mm512_and_epi64(parent1_ptr[i], mask)), (_mm512_andnot_epi64(parent2_ptr[i], mask)));
	}
#else 
#ifdef __AVX2__
	__m256i mask;
	uint32_t memory_blocks = individual_mem_size / sizeof(__m256i);

	__m256i* parent1_ptr = (__m256i*)parent1;
	__m256i* parent2_ptr = (__m256i*)parent2;
	__m256i* child1_ptr = (__m256i*)child1;
	__m256i* child2_ptr = (__m256i*)child2;

	for (int i = 0; i < memory_blocks; i++) {
		mask = gen_mt_rand256();

		child1_ptr[i] = _mm256_or_epi64((_mm256_andnot_epi64(parent1_ptr[i], mask)), (_mm256_and_epi64(parent2_ptr[i], mask)));
		child2_ptr[i] = _mm256_or_epi64((_mm256_and_epi64(parent1_ptr[i], mask)), (_mm256_andnot_epi64(parent2_ptr[i], mask)));
	}
#else
	int mask;

	for (int i = 0; i < genes; i++) {

		mask = gen_mt_rand();

		child1[i] = (parent1[i] & ~mask) | (parent2[i] & mask);
		child2[i] = (parent1[i] & mask) | (parent2[i] & ~mask);
	}
#endif
#endif
}

static void complete_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// The function should fill child1 and child2 with the crossed over values

	// int mask = pow(2, point) - 1;

	for (int i = 0; i < genes; i++) {
		if (gen_mt_rand() % 2 == 0) {
			child1[i] = parent1[i];
			child2[i] = parent2[i];
		}
		else {
			child1[i] = parent2[i];
			child2[i] = parent1[i];
		}
	}
}

static void crossover(int* parent1, int* parent2, int* child1, int* child2, int genes, int individual_mem_size, crossover_param_t* crossover_param) {

	if (crossover_param->crossover_method == crossover_method_single_pointAVX) {
		single_point_crossoverAVX(parent1, parent2, child1, child2, genes, individual_mem_size);
	}
	else if (crossover_param->crossover_method == crossover_method_two_point32) {
		two_point_crossoverAVX(parent1, parent2, child1, child2, genes, individual_mem_size);
	}
	else if (crossover_param->crossover_method == crossover_method_uniform32) {
		uniform_crossover32(parent1, parent2, child1, child2, genes);
	}
	else if (crossover_param->crossover_method == crossover_method_complete32) {
		complete_crossover32(parent1, parent2, child1, child2, genes);
	}
    else if (crossover_param->crossover_method == crossover_method_uniformAVX) {
        uniform_crossoverAVX(parent1, parent2, child1, child2, genes, individual_mem_size);
    }
	else {
		printf("Invalid crossover method\n");
	}

}

void process_crossover(gene_pool_t* gene_pool, crossover_param_t* crossover_param) {
	//double** pop_parameter_bin, int individuals, int genes, int* selected, int skipped_pairs){
	int next_even = (gene_pool->individuals - gene_pool->elitism) + ((gene_pool->individuals - gene_pool->elitism) % 2);

	for (int i = 0; i < next_even; i += 2) {
		crossover(gene_pool->pop_param_bin[gene_pool->selected_indexes[i]],
			gene_pool->pop_param_bin[gene_pool->selected_indexes[i + 1]],
			gene_pool->pop_param_bin_cross_buffer[i],
			gene_pool->pop_param_bin_cross_buffer[i + 1],
			gene_pool->genes,
            gene_pool->individual_mem_size,
			crossover_param
		);
	}

	// copy the crossed over values back to the population
	for (int i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
        memcpy(gene_pool->pop_param_bin[gene_pool->sorted_indexes[i]],
			gene_pool->pop_param_bin_cross_buffer[i],
			gene_pool->individual_mem_size);
	}
}


