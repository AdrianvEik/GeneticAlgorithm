
#include <math.h>

#include "../Optimisation/Optimizer.h"

void new_adaptive_memory(adaptive_memory_t* adaptive_memory) {
    adaptive_memory->iteration_counter = -1;
    adaptive_memory->convergence_counter = 0;
    adaptive_memory->convergence_reached = 0;
    adaptive_memory->computed_mutation = 0;
    adaptive_memory->convergence_moving_window_alpha = 0;
    adaptive_memory->convergence_moving_window_beta = 0;

    adaptive_memory->group_dispersion = 0;
    adaptive_memory->group_dispersion_alpha = 0;
    adaptive_memory->group_dispersion_beta = 0;
    adaptive_memory->group_dispersion_moving_window = 0;
    adaptive_memory->computed_flatten_factor = 0;
    adaptive_memory->computed_flatten_bias = 0;

    adaptive_memory->convergence_moving_window = 0;
    adaptive_memory->previous_best_result = 0;
}

void check_convergence(task_param_t* task, adaptive_memory_t* adaptive_memory, double best_result) {
    adaptive_memory->iteration_counter++;
    if (adaptive_memory->iteration_counter > task->config_ga.optimizer_param.max_iterations) {
        adaptive_memory->convergence_reached = 1;
        printf("Max iterations reached: %d\n", adaptive_memory->iteration_counter);
    }

    if (fabs(best_result - adaptive_memory->previous_best_result) < task->config_ga.optimizer_param.convergence_threshold) {
        adaptive_memory->convergence_counter++;
        if (adaptive_memory->convergence_counter > task->config_ga.optimizer_param.convergence_window) {
            adaptive_memory->convergence_reached = 1;
            printf("Converged at iteration: %d\n", adaptive_memory->iteration_counter);
        }
    }
    else {
        adaptive_memory->convergence_counter = 0;
    }
    adaptive_memory->previous_best_result = best_result;
}

void compute_mutation_rate(task_param_t* task, adaptive_memory_t* adaptive_memory, double best_result) {
    int computed_mutation;

    if (adaptive_memory->convergence_moving_window == 0) {
        if (task->config_ga.mutation_param.mutation_rate < task->config_ga.optimizer_param.max_mutations) {
            task->config_ga.mutation_param.mutation_rate++;
        }
    }
    else {
        // TODO: check if log is correct
        adaptive_memory->computed_mutation = sqrt(task->config_ga.optimizer_param.convergence_threshold / (best_result - adaptive_memory->convergence_moving_window)) * task->config_ga.optimizer_param.mutation_factor;
        if (adaptive_memory->computed_mutation < INT32_MAX) {
            computed_mutation = (int)adaptive_memory->computed_mutation;
        }
        else {
            computed_mutation = INT32_MAX;
        }

        if (computed_mutation < task->config_ga.optimizer_param.min_mutations) {
            task->config_ga.mutation_param.mutation_rate = task->config_ga.optimizer_param.min_mutations;
        }
        else if (computed_mutation > task->config_ga.optimizer_param.max_mutations) {
            task->config_ga.mutation_param.mutation_rate = task->config_ga.optimizer_param.max_mutations;
        }
        else {
            task->config_ga.mutation_param.mutation_rate = computed_mutation;
        }
    }
}

void compute_flatten_factors(task_param_t* task, adaptive_memory_t* adaptive_memory, double best_result) {
    // Based on dispersion of the group compared to the best result and the average result of the group
    // calculate the flatten factor and bias


}


void adapt_param(task_param_t* task, gene_pool_t* gene_pool, adaptive_memory_t* adaptive_memory) {
	
	double best_result = gene_pool->pop_result_set[gene_pool->sorted_indexes[gene_pool->individuals - 1]];

	// Check for convergence & runtime params
    check_convergence(task, adaptive_memory, best_result);


	if (adaptive_memory->iteration_counter == 0) {
        adaptive_memory->convergence_moving_window_alpha = ((double) task->config_ga.optimizer_param.convergence_moving_window_size-1)/ (double) task->config_ga.optimizer_param.convergence_moving_window_size;
        adaptive_memory->convergence_moving_window_beta = 1 / (double) task->config_ga.optimizer_param.convergence_moving_window_size;
	}

    // Compute mutation rate
    compute_mutation_rate(task, adaptive_memory, best_result);

    // Compute flatten factor
}