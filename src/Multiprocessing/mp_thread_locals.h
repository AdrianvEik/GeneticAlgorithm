
#ifndef MP_THREAD_LOCALS_H
#define MP_THREAD_LOCALS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"

#define thread_local __declspec( thread )

/**
 * Allocate all thread-local pre-compute buffers for a solver thread.
 *
 * This delegates to selection-specific setup and prepares scratch arrays used
 * during population processing.
 *
 * :param gene_pool: Population dimensions used for buffer sizes.
 */
void init_pre_compute(gene_pool_t* gene_pool);

/**
 * Release all thread-local pre-compute buffers for the current solver thread.
 */
void free_pre_compute();

/** Lazily allocate/reuse rank and optional diversity workspaces for this size. */
void prepare_selection_workspace(gene_pool_t* gene_pool, int need_diversity);
void free_pre_compute_selection(void);

// RNG
// has a thread local seed struct for each thread

// Selection parameters
/** Cached geometric weights in ascending rank order; prepared only for rank methods. */
extern thread_local double* prob_distr;
/** Cached exponential rank weights (not the three-candidate selector). */
extern thread_local double* boltzmann_distr;
/** Last selection probability parameter used to decide whether to recompute. */
extern thread_local double current_prob_param;
/** Last Boltzmann temperature parameter used to decide whether to recompute. */
extern thread_local double current_temp_param;
/** Population-specific tie-adjusted rank weights, separate from cached bases. */
extern thread_local double* rank_weights;
/** Diversity cumulative distribution in physical individual order. */
extern thread_local double* diversity_cdf;

// In case of using the rank_space selection method
/** Per-individual distances from the population central point. */
extern thread_local double* distances;
/** Central point used by rank-space selection. */
extern thread_local double* central_point;

// Mutation parameters
/** Reserved scratch distribution for mutation boosting. */
extern thread_local int* muation_boost_distr;
/** Reserved adaptive mutation alpha parameter cache. */
extern thread_local double current_alpha;
/** Reserved adaptive mutation beta parameter cache. */
extern thread_local double current_beta;

// Population index sequence
extern thread_local uint32_t* pop_index_sequence;

// Mutation selection array for gene-level mutation
extern thread_local uint32_t* mutation_selection_array;

#endif // !MP_THREAD_LOCALS_H
