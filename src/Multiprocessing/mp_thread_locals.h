
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

// RNG
// has a thread local seed struct for each thread

// Selection parameters
/** Cached probability distribution used by roulette and rank selection. */
extern thread_local double* prob_distr;
/** Cached Boltzmann probability distribution used by Boltzmann selection. */
extern thread_local double* boltzmann_distr;
/** Last selection probability parameter used to decide whether to recompute. */
extern thread_local double current_prob_param;
/** Last Boltzmann temperature parameter used to decide whether to recompute. */
extern thread_local double current_temp_param;

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

#endif // !MP_THREAD_LOCALS_H
