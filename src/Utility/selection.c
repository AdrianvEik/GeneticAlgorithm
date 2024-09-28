#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"

#include "selection.h"

// Selection functions
void roulette_selection(gene_pool_t* gene_pool, selection_param_t* selection_param, mt_rand_t* mt_rand) {
	/*

	:param pop: matrix of normalised fitness values for the population (individuals x 1)
	:param individuals: number of individuals
	:param genes: number of genes
	:param selected: matrix of the indices of selected individuals (individuals x 2)

	*/

	// select individuals
	roulette_wheel(gene_pool->flatten_result_set, gene_pool->individuals, gene_pool->individuals - gene_pool->elitism, gene_pool->selected_indexes, mt_rand);
}

void rank_tournament_selection(gene_pool_t* gene_pool, selection_param_t* selection_param, mt_rand_t* mt_rand) {
	/*

	:param pop: matrix of individuals as double (individuals x genes)
	:param individuals: number of individuals
	:param genes: number of genes
	:param fx: fitness function (double array x0 x1 ... xn)
	:param flatten: flattening function (double array, int, double, double, double array)
	:param a: parameter for flattening function
	:param b: parameter for flattening function
	:param mode: 0 for minimisation, 1 for maximisation
	:param result: matrix of the indices of selected individuals (individuals x 2)

	*/


}
void rank_selection(gene_pool_t* gene_pool, selection_param_t* selection_param, mt_rand_t* mt_rand) {
	/*

	:param pop: matrix of individuals as double (individuals x genes)
	:param individuals: number of individuals
	:param genes: number of genes
	:param fx: fitness function (double array x0 x1 ... xn)
	:param flatten: flattening function (double array, int, double, double, double array)
	:param a: parameter for flattening function
	:param b: parameter for flattening function
	:param mode: 0 for minimisation, 1 for maximisation
	:param result: matrix of the indices of selected individuals (individuals x 2)

	*/


}
void rank_space_selection(gene_pool_t* gene_pool, selection_param_t* selection_param, mt_rand_t* mt_rand) {
	/*

	:param pop: matrix of individuals as double (individuals x genes)
	:param individuals: number of individuals
	:param genes: number of genes
	:param fx: fitness function (double array x0 x1 ... xn)
	:param flatten: flattening function (double array, int, double, double, double array)
	:param a: parameter for flattening function
	:param b: parameter for flattening function
	:param mode: 0 for minimisation, 1 for maximisation
	:param result: matrix of the indices of selected individuals (individuals x 2)

	*/

}
void boltzmann_selection(gene_pool_t* gene_pool, selection_param_t* selection_param, mt_rand_t* mt_rand) {
	/*

	:param pop: matrix of individuals as double (individuals x genes)
	:param individuals: number of individuals
	:param genes: number of genes
	:param fx: fitness function (double array x0 x1 ... xn)
	:param flatten: flattening function (double array, int, double, double, double array)
	:param a: parameter for flattening function
	:param b: parameter for flattening function
	:param mode: 0 for minimisation, 1 for maximisation
	:param result: matrix of the indices of selected individuals (individuals x 2)

	*/

}



void process_selection(gene_pool_t* gene_pool, selection_param_t* selection_param, mt_rand_t* mt_rand) {
	if (selection_param->selection_method == selection_method_roulette) {
		roulette_selection(gene_pool, selection_param, mt_rand);
	}
	else if (selection_param->selection_method == selection_method_rank_tournament) {
		rank_tournament_selection(gene_pool, selection_param, mt_rand);
	}
	else if (selection_param->selection_method == selection_method_rank) {
		rank_selection(gene_pool, selection_param, mt_rand);
	}
	else if (selection_param->selection_method == selection_method_rank_space) {
		rank_space_selection(gene_pool, selection_param, mt_rand);
	}
	else if (selection_param->selection_method == selection_method_boltzmann) {
		boltzmann_selection(gene_pool, selection_param, mt_rand);
	}
	else {
		printf("Error: selection_method is not 0, 1, 2, 3 or 4\n");
	}
}