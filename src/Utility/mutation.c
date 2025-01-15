#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "mutation.h"
#include "../Multiprocessing/mp_thread_locals.h"


void compute_mutation_distr(gene_pool_t* gene_pool, mutation_param_t* mutation_param) {
    /*
    */
    double sum = 0;
    current_alpha = mutation_param->mutation_alpha;
    current_beta = mutation_param->mutation_beta;
    for (int i = 0; i < gene_pool->individuals; i++) {
        muation_boost_distr[i] = 0;
        sum += muation_boost_distr[i];
    }
    for (int i = 0; i < gene_pool->individuals; i++) {
        muation_boost_distr[i] /= sum;
    }
}

void mutate32(gene_pool_t* gene_pool, mutation_param_t* mutation_param) {

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

	int mutation_gene;
	int mutation_bit = 0;
	for (int i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
		for (int j = 0; j < mutation_param->mutation_rate; j++) { // check if works
			// ensure that the selected gene is positive
			mutation_gene = gen_mt_rand() % gene_pool->genes;
			mutation_bit = (int)1 << (gen_mt_rand() % sizeof(int) * 8); // mask
			gene_pool->pop_param_bin[gene_pool->sorted_indexes[i]][mutation_gene] ^= mutation_bit;
		}
	}

}


void process_mutation(gene_pool_t* gene_pool, mutation_param_t* mutation_param) {
    /*
    */
    // Check if the distributions are up to date

	// to Optimizer.c?
    if (current_alpha != mutation_param->mutation_alpha ||
        current_beta != mutation_param->mutation_beta ||
		muation_boost_distr[0] == -1) { // Change or not initialized
        compute_mutation_distr(gene_pool, mutation_param);
    }



}