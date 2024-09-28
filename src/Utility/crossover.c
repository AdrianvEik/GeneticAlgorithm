#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "crossover.h"
#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"


// Path: Utility/crossover.c

void single_point_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes, mt_rand_t* mt_rand) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// The function should fill child1 and child2 with the crossed over values

	// find a random point to cross over
	int point = gen_mt_rand(mt_rand) % (genes * sizeof(int) * 8 - 1);

	// int mask = pow(2, point) - 1;

	int mask = 0;
	int bit_i = 0;
	for (int i = 0; i < genes; i++) {
		bit_i = i * sizeof(int) * 8;

		if (bit_i < point - sizeof(int) * 8) {
			mask = 0x0;
		}
		else if (bit_i < point) {
			mask = (1 << (bit_i - point)) - 1;
		}
		else {
			mask = 0xffffffff;
		}

		child1[i] = (parent1[i] & ~mask) | (parent2[i] & mask);
		child2[i] = (parent1[i] & mask) | (parent2[i] & ~mask);

	}

	// // copy the first part of parent1 to child1 and the first part of parent2 to child2
	// for(int i; i<point; i++){
	//     child1[i] = parent1[i];
	//     child2[i] = parent2[i];
	// }
	// // for the other half, copy the values from the other parent
	// for(int i; i<size; i++){
	//     child1[i] = parent2[i];
	//     child2[i] = parent1[i];
	// }
}

void two_point_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes, mt_rand_t* mt_rand) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// point1 and point2 are the points to cross over at
	// The function should fill child1 and child2 with the crossed over values

	// find two random points to cross over

	int point1 = gen_mt_rand(mt_rand) % genes * sizeof(int) * 8 - 3;
	int point2 = gen_mt_rand(mt_rand) % (genes * sizeof(int) * 8 - 2 - point1) + point1 + 1;

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

void uniform_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes, mt_rand_t* mt_rand) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// prob is the probability of a value being copied from the first parent
	// The function should fill child1 and child2 with the crossed over values

	// int mask = pow(2, point) - 1;

	int mask;

	for (int i = 0; i < genes; i++) {

		mask = gen_mt_rand(mt_rand);

		child1[i] = (parent1[i] & ~mask) | (parent2[i] & mask);
		child2[i] = (parent1[i] & mask) | (parent2[i] & ~mask);
	}
}

void complete_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes, mt_rand_t* mt_rand) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// The function should fill child1 and child2 with the crossed over values

	// int mask = pow(2, point) - 1;

	for (int i = 0; i < genes; i++) {
		if (gen_mt_rand(mt_rand) % 2 == 0) {
			child1[i] = parent1[i];
			child2[i] = parent2[i];
		}
		else {
			child1[i] = parent2[i];
			child2[i] = parent1[i];
		}
	}
}

void crossover(int* parent1, int* parent2, int* child1, int* child2, int genes, crossover_param_t* crossover_param, mt_rand_t *mt_rand) {

	if (crossover_param->crossover_method == crossover_method_single_point32) {
		single_point_crossover32(parent1, parent2, child1, child2, genes, mt_rand);
	}
	else if (crossover_param->crossover_method == crossover_method_two_point32) {
		two_point_crossover32(parent1, parent2, child1, child2, genes, mt_rand);
	}
	else if (crossover_param->crossover_method == crossover_method_uniform32) {
		uniform_crossover32(parent1, parent2, child1, child2, genes, mt_rand);
	}
	else if (crossover_param->crossover_method == crossover_method_complete32) {
		complete_crossover32(parent1, parent2, child1, child2, genes, mt_rand);
	}
	else {
		printf("Invalid crossover method\n");
	}

}

void process_crossover(gene_pool_t* gene_pool, crossover_param_t* crossover_param, mt_rand_t* mt_rand) {
	//double** pop_parameter_bin, int individuals, int genes, int* selected, int skipped_pairs){
	int nearest_even = (gene_pool->individuals - gene_pool->elitism) - ((gene_pool->individuals - gene_pool->elitism) % 2);

	for (int i = 0; i < nearest_even; i += 2) {
		crossover(gene_pool->pop_param_bin[gene_pool->selected_indexes[i]],
			gene_pool->pop_param_bin[gene_pool->selected_indexes[i + 1]],
			gene_pool->pop_param_bin_cross_buffer[i],
			gene_pool->pop_param_bin_cross_buffer[i + 1],
			gene_pool->genes,
			crossover_param,
			mt_rand);
	}

	int skip_index = 0;

	// copy the crossed over values back to the population
	for (int i = 0; i < nearest_even; i++) {

		skip_index = 0;
		for (int k = nearest_even; k < gene_pool->individuals - 1; k++) {
			if (gene_pool->selected_indexes[k] == i) {
				// continue outer loop;
				skip_index = 1;
				break;
			}
		}
		if (skip_index) {
			for (int j = 0; j < gene_pool->genes; j++) {
				gene_pool->pop_param_bin[gene_pool->sorted_indexes[i]][j] = gene_pool->pop_param_bin_cross_buffer[i][j];
			}
		}
	}
}


