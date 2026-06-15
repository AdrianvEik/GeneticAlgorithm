
#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <math.h>
#include <stdint.h>

#include "../Helper/Struct.h"
#include "../Multiprocessing/mp_solver_th.h"

/**
 * Adaptive state carried across iterations for one solver task.
 *
 * The optimizer stores convergence counters, moving-window estimates, and the
 * normalized mutation signal used to update per-individual mutation rates.
 */
struct adaptive_memory_s {
    // Mutation
	/** Exponential moving value of the best result. */
	double convergence_moving_window;
	/** Alpha coefficient for the convergence moving window. */
	double convergence_moving_window_alpha;
	/** Reserved beta coefficient for future convergence fitting. */
	double convergence_moving_window_beta;
	/** Best result from the previous iteration. */
	double previous_best_result;
    /** Normalized mutation signal computed from convergence progress. */
    double computed_mutation;
	/** Minimum computed mutation signal seen by this task. */
	double computed_mutation_min_seen;
    /** Maximum computed mutation signal seen by this task. */
    double computed_mutation_max_seen;

    // Flatten
	/** Reserved population dispersion metric for adaptive flattening. */
	double group_dispersion;
	/** Reserved alpha coefficient for adaptive flattening. */
	double group_dispersion_alpha;
	/** Reserved beta coefficient for adaptive flattening. */
	double group_dispersion_beta;
	/** Reserved moving window for population dispersion. */
	double group_dispersion_moving_window;
	/** Reserved computed flattening scale. */
	double computed_flatten_factor;
	/** Reserved computed flattening bias. */
	double computed_flatten_bias;

	//uint32_t iteration_counter;
	/** Count of consecutive iterations within the convergence threshold. */
	uint32_t convergence_counter;
	/** Non-zero once convergence or the iteration cap has stopped the task. */
	int convergence_reached;
};

typedef struct adaptive_memory_s adaptive_memory_t;

/**
 * Initialize adaptive optimizer state before processing a task.
 *
 * :param adaptive_memory: Adaptive-memory instance to reset.
 */
void new_adaptive_memory(adaptive_memory_t* adaptive_memory);

/**
 * Update adaptive mutation settings and convergence state.
 *
 * The function reads the current best sorted result from ``gene_pool``, updates
 * moving-window convergence data, adjusts ``task->config_ga.mutation_param``,
 * and sets ``convergence_reached`` when the task should stop.
 *
 * :param task: Current task whose configuration is adapted in-place.
 * :param gene_pool: Population state containing sorted fitness results.
 * :param adaptive_memory: State carried between iterations of the same task.
 */
void adapt_param(task_param_t* task, gene_pool_t* gene_pool, adaptive_memory_t* adaptive_memory);

#endif // OPTIMIZER_H
