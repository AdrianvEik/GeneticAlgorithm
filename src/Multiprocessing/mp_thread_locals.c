#include "mp_thread_locals.h"
#include <math.h>

thread_local double* prob_distr = NULL;
thread_local double* boltzmann_distr = NULL;
thread_local double current_prob_param = NAN;
thread_local double current_temp_param = NAN;
thread_local double* rank_weights = NULL;
thread_local double* diversity_cdf = NULL;
thread_local double* distances = NULL;
thread_local double* central_point = NULL;
static thread_local uint32_t selection_size = 0;
static thread_local uint32_t diversity_size = 0;
static thread_local uint32_t diversity_genes = 0;
thread_local uint32_t* pop_index_sequence = NULL;
thread_local uint32_t* mutation_selection_array = NULL;

/* Lazy, reusable selection storage; all caches are invalidated on size changes. */
void prepare_selection_workspace(gene_pool_t* pool, int need_diversity) {
    if (selection_size != pool->individuals || rank_weights == NULL) {
        free(prob_distr);
        free(boltzmann_distr);
        free(rank_weights);
        const size_t bytes = (size_t)pool->individuals * sizeof(double);
        prob_distr = (double*)malloc(bytes);
        boltzmann_distr = (double*)malloc(bytes);
        rank_weights = (double*)malloc(bytes);
        if (!prob_distr || !boltzmann_distr || !rank_weights) EXIT_MEM_ERROR();
        selection_size = pool->individuals;
        current_prob_param = NAN;
        current_temp_param = NAN;
    }
    if (need_diversity && (diversity_size != pool->individuals ||
                          diversity_genes != pool->genes || distances == NULL)) {
        free(distances);
        free(diversity_cdf);
        free(central_point);
        distances = (double*)malloc((size_t)pool->individuals * sizeof(double));
        diversity_cdf = (double*)malloc((size_t)pool->individuals * sizeof(double));
        central_point = (double*)malloc((size_t)pool->genes * sizeof(double));
        if (!distances || !diversity_cdf || !central_point) EXIT_MEM_ERROR();
        diversity_size = pool->individuals;
        diversity_genes = pool->genes;
    }
}

void init_pre_compute_selection(gene_pool_t* pool) {
    free_pre_compute_selection();
    pop_index_sequence = (uint32_t*)malloc((size_t)pool->individuals * sizeof(uint32_t));
    mutation_selection_array = (uint32_t*)malloc((size_t)pool->genes * sizeof(uint32_t));
    if (!pop_index_sequence || !mutation_selection_array) EXIT_MEM_ERROR();
    for (uint32_t i = 0; i < pool->individuals; ++i) pop_index_sequence[i] = i;
}

void free_pre_compute_selection(void) {
    free(prob_distr); prob_distr = NULL;
    free(boltzmann_distr); boltzmann_distr = NULL;
    free(rank_weights); rank_weights = NULL;
    free(distances); distances = NULL;
    free(diversity_cdf); diversity_cdf = NULL;
    free(central_point); central_point = NULL;
    free(pop_index_sequence); pop_index_sequence = NULL;
    free(mutation_selection_array); mutation_selection_array = NULL;
    selection_size = diversity_size = diversity_genes = 0;
    current_prob_param = current_temp_param = NAN;
}

void init_pre_compute(gene_pool_t* pool) { init_pre_compute_selection(pool); }
void free_pre_compute(void) { free_pre_compute_selection(); }
