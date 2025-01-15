
#include <stdlib.h>
#include "mp_thread_locals.h"

void init_pre_compute_selection(gene_pool_t* gene_pool) {
    /*
    */
    prob_distr = (double*)malloc(gene_pool->individuals * sizeof(double));
    boltzmann_distr = (double*)malloc(gene_pool->individuals * sizeof(double));

    if (prob_distr == NULL || boltzmann_distr == NULL) {
        printf("Memory allocation failed");
        exit(255);
    }

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