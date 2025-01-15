
#include <stdlib.h>
#include <string.h>
#include "mp_thread_locals.h"
#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"


// Selection parameters
thread_local double* prob_distr = NULL;
thread_local double* boltzmann_distr = NULL;
thread_local double current_prob_param = 0.0;
thread_local double current_temp_param = 0.0;

// In case of using the rank_space selection method
thread_local double* distances = NULL;
thread_local double* central_point = NULL;

// Mutation parameters
thread_local int* muation_boost_distr = NULL;
thread_local double current_alpha = 0.0;
thread_local double current_beta = 0.0;

void init_pre_compute_selection(gene_pool_t* gene_pool) {
    /*
    */
    prob_distr = (double*)malloc(gene_pool->individuals * sizeof(double));
    boltzmann_distr = (double*)malloc(gene_pool->individuals * sizeof(double));

    if (prob_distr == NULL || boltzmann_distr == NULL) EXIT_MEM_ERROR();

    memset(prob_distr, -1, gene_pool->individuals * sizeof(double));
    memset(boltzmann_distr, -1, gene_pool->individuals * sizeof(double));
}

// This needs to be called externally to free the memory
void free_pre_compute_selection() {
    /*
    */
    free(prob_distr);
    free(boltzmann_distr);

    // They are malloc in pairs
    if (distances != NULL && central_point != NULL) {
        free(distances);
        free(central_point);
    }
}

void init_pre_compute(gene_pool_t* gene_pool) {
    init_pre_compute_selection(gene_pool);
}

void free_pre_compute() {
    free_pre_compute_selection();
}