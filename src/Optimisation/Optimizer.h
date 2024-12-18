
#include "../Helper/multiprocessing.h" 
#include "../Helper/struct.h"

#ifndef OPTIMIZER_H
#define OPTIMIZER_H


struct adaptive_memory_s {
    // Mutation
	double convergence_moving_window;
	double convergence_moving_window_alpha;
	double convergence_moving_window_beta;
	double previous_best_result;
    double computed_mutation;

    // Flatten
	double group_dispersion;
	double group_dispersion_alpha;
	double group_dispersion_beta;
	double group_dispersion_moving_window;
	double computed_flatten_factor;
	double computed_flatten_bias;

	int iteration_counter;
	int convergence_counter;
	int convergence_reached;
};

typedef struct adaptive_memory_s adaptive_memory_t;

void new_adaptive_memory(adaptive_memory_t* adaptive_memory);

void adapt_param(task_param_t* task, gene_pool_t* gene_pool, adaptive_memory_t* adaptive_memory);

#endif // OPTIMIZER_H