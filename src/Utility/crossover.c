#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "crossover.h"
#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/intel_rand.h"


// Path: Utility/crossover.c

void single_point_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// The function should fill child1 and child2 with the crossed over values

	// find a random point to cross over
	int point = rand() % (genes * sizeof(int) * 8 - 1);

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

void two_point_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// point1 and point2 are the points to cross over at
	// The function should fill child1 and child2 with the crossed over values

	// find two random points to cross over

	int point1 = rand() % genes * sizeof(int) * 8 - 3;
	int point2 = rand() % (genes * sizeof(int) * 8 - 2 - point1) + point1 + 1;

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

void uniform_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// prob is the probability of a value being copied from the first parent
	// The function should fill child1 and child2 with the crossed over values

	// int mask = pow(2, point) - 1;

	int mask;

	for (int i = 0; i < genes; i++) {

		mask = rdrand();

		child1[i] = (parent1[i] & ~mask) | (parent2[i] & mask);
		child2[i] = (parent1[i] & mask) | (parent2[i] & ~mask);
	}
}

void complete_crossover32(int* parent1, int* parent2, int* child1, int* child2, int genes) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// The function should fill child1 and child2 with the crossed over values

	// int mask = pow(2, point) - 1;

	for (int i = 0; i < genes; i++) {
		if (rand() % 2 == 0) {
			child1[i] = parent1[i];
			child2[i] = parent2[i];
		}
		else {
			child1[i] = parent2[i];
			child2[i] = parent1[i];
		}
	}
}


// Below are the variable bit size crossover functions

void single_point_crossover(int* parent1, int* parent2, int* child1, int* child2, int genes, int bitsize) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// The function should fill child1 and child2 with the crossed over values

	int size = genes * bitsize;

	// find a random point to cross over
	int point = rand() % size;

	// copy the first part of parent1 to child1 and the first part of parent2 to child2
	for (int i = 0; i < point; i++) {
		child1[i] = parent1[i];
		child2[i] = parent2[i];
	}
	// for the other half, copy the values from the other parent
	for (int i = 0; point < size; i++) {
		child1[i] = parent2[i];
		child2[i] = parent1[i];
	}
}

void two_point_crossover(int* parent1, int* parent2, int* child1, int* child2, int genes, int bitsize) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// point1 and point2 are the points to cross over at
	// The function should fill child1 and child2 with the crossed over values

	int size = genes * bitsize;

	// find two random points to cross over
	int point1 = rand() % size - 1;
	int point2 = rand() % (size - point1) + point1;

	// make sure point1 is less than point2 and less than size
	if (point1 > point2 || point2 > size) {
		int temp = point1;
		point1 = point2;
		point2 = temp;

		if (point2 > size) {
			point2 = size - 1;
		}
	}

	// copy the first part of parent1 to child1 and the first part of parent2 to child2
	for (int i = 0; i < point1; i++) {
		child1[i] = parent2[i];
		child2[i] = parent1[i];
	}
	// for the other half, copy the values from the other parent
	for (int i = point1; i < point2; i++) {
		child1[i] = parent1[i];
		child2[i] = parent2[i];
	}
	// for the last part, copy the values from the other parent
	for (int i = point2; i < size; i++) {
		child1[i] = parent2[i];
		child2[i] = parent1[i];
	}
}

void uniform_crossover(int* parent1, int* parent2, int* child1, int* child2, int genes, int bitsize) {
	// parent1 and parent2 are the parents to be crossed over and child1 and child2 are the children to be created all of size size
	// prob is the probability of a value being copied from the first parent
	// The function should fill child1 and child2 with the crossed over values

	// for each value, copy from the first parent with probability prob
	for (int i = 0; i < genes * bitsize; i++) {
		if (rand() % 2) {
			child1[i] = parent1[i];
			child2[i] = parent2[i];
		}
		else {
			child1[i] = parent2[i];
			child2[i] = parent1[i];
		}
	}
}

void complete_crossover(int* parent1, int* parent2, int* child1, int* child2, int genes, int bitsize) {


	for (int i = 0; i < genes * bitsize; i += bitsize) {
		if (rand() % 100 < 50) {
			for (int j = 0; j < bitsize; j++) {
				child1[i + j] = parent1[i + j];
				child2[i + j] = parent2[i + j];
			}
		}
		else {
			for (int j = 0; j < bitsize; j++) {
				child1[i + j] = parent2[i + j];
				child2[i + j] = parent1[i + j];
			}
		}
	}
}

void crossover(int* parent1, int* parent2, int* child1, int* child2, int genes, crossover_param_t* crossover_param) {

	if (crossover_param->crossover_method == crossover_method_single_point) {
		single_point_crossover(parent1, parent2, child1, child2, genes, 32); // TODO: bitsize
	}
	else if (crossover_param->crossover_method == crossover_method_two_point) {
		two_point_crossover(parent1, parent2, child1, child2, genes, 32);
	}
	else if (crossover_param->crossover_method == crossover_method_uniform) {
		uniform_crossover(parent1, parent2, child1, child2, genes, 32);
	}
	else if (crossover_param->crossover_method == crossover_method_complete) {
		complete_crossover(parent1, parent2, child1, child2, genes, 32);
	}
	else if (crossover_param->crossover_method == crossover_method_single_point32) {
		single_point_crossover32(parent1, parent2, child1, child2, genes);
	}
	else if (crossover_param->crossover_method == crossover_method_two_point32) {
		two_point_crossover32(parent1, parent2, child1, child2, genes);
	}
	else if (crossover_param->crossover_method == crossover_method_uniform32) {
		uniform_crossover32(parent1, parent2, child1, child2, genes);
	}
	else if (crossover_param->crossover_method == crossover_method_complete32) {
		complete_crossover32(parent1, parent2, child1, child2, genes);
	}
	else {
		printf("Invalid crossover method\n");
	}

}

void process_crossover(gene_pool_t* gene_pool, crossover_param_t* crossover_param) {
	//double** pop_parameter_bin, int individuals, int genes, int* selected, int skipped_pairs){
	int nearest_even = (gene_pool->individuals - gene_pool->elitism) - ((gene_pool->individuals - gene_pool->elitism) % 2);

	for (int i = 0; i < nearest_even; i += 2) {
		crossover(gene_pool->pop_param_bin[gene_pool->selected_indexes[i]],
			gene_pool->pop_param_bin[gene_pool->selected_indexes[i + 1]],
			gene_pool->pop_param_bin_cross_buffer[i],
			gene_pool->pop_param_bin_cross_buffer[i + 1],
			gene_pool->genes,
			crossover_param);
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


