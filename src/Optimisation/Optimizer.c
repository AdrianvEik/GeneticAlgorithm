
#include "Optimizer.h"

void new_adaptive_memory(adaptive_memory_t* adaptive_memory) {
    adaptive_memory->iteration_counter = -1;
    adaptive_memory->convergence_counter = 0;
    adaptive_memory->convergence_reached = 0;
    adaptive_memory->computed_mutation = 0;
    adaptive_memory->computed_mutation_min_seen = INT32_MAX;
    adaptive_memory->computed_mutation_max_seen = 0;
    adaptive_memory->convergence_moving_window_alpha = 0;
    adaptive_memory->convergence_moving_window_beta = 0;

    // TODO: adaptive flattening, alpha/beta fitting, remember worst answer
    adaptive_memory->group_dispersion = 0;
    adaptive_memory->group_dispersion_alpha = 0;
    adaptive_memory->group_dispersion_beta = 0;
    adaptive_memory->group_dispersion_moving_window = 0;
    adaptive_memory->computed_flatten_factor = 0;
    adaptive_memory->computed_flatten_bias = 0;

    adaptive_memory->convergence_moving_window = 0;
    adaptive_memory->previous_best_result = 0;
}

static void check_convergence(task_param_t* task, adaptive_memory_t* adaptive_memory, double best_result) {
    adaptive_memory->iteration_counter++;
    if (adaptive_memory->iteration_counter > task->config_ga.optimizer_param.max_iterations) {
        adaptive_memory->convergence_reached = 1;
    }

    if (fabs(best_result - adaptive_memory->previous_best_result) < task->config_ga.optimizer_param.convergence_threshold) {
        adaptive_memory->convergence_counter++;
        if (adaptive_memory->convergence_counter > task->config_ga.optimizer_param.convergence_window) {
            adaptive_memory->convergence_reached = 1;
        }
    }
    else {
        adaptive_memory->convergence_counter = 0;
    }
    adaptive_memory->convergence_moving_window = adaptive_memory->convergence_moving_window_alpha * best_result + (1 - adaptive_memory->convergence_moving_window_alpha) * adaptive_memory->convergence_moving_window;

    adaptive_memory->previous_best_result = best_result;
}

static void compute_mutation_rate(task_param_t* task, adaptive_memory_t* adaptive_memory, double best_result, uint32_t individuals) {
    double computed_mutation = 0.0;
    double computed_mutation_sloped = 0.0;
    adaptive_memory->computed_mutation = task->config_ga.optimizer_param.convergence_threshold / (best_result - adaptive_memory->convergence_moving_window);
    if (adaptive_memory->computed_mutation < INT32_MAX) {
        computed_mutation = adaptive_memory->computed_mutation;
        if (computed_mutation < adaptive_memory->computed_mutation_min_seen) {
            adaptive_memory->computed_mutation_min_seen = computed_mutation;
            computed_mutation = 0;
        }
        else if (computed_mutation > adaptive_memory->computed_mutation_max_seen) {
            adaptive_memory->computed_mutation_max_seen = computed_mutation;
            computed_mutation = 1;
        }
        else {
            // scale to min/max mutations
            computed_mutation = (computed_mutation - adaptive_memory->computed_mutation_min_seen) / (adaptive_memory->computed_mutation_max_seen - adaptive_memory->computed_mutation_min_seen);
        }
    }
    else {
        computed_mutation = task->config_ga.optimizer_param.max_mutations;
    }

    // cleanup
    adaptive_memory->computed_mutation = computed_mutation;

    for (uint32_t i = 0; i < individuals; i++) {
        if (adaptive_memory->convergence_moving_window == 0) {
            if (task->config_ga.mutation_param.mutation_rate[i] < task->config_ga.optimizer_param.max_mutations) {
                task->config_ga.mutation_param.mutation_rate[i]++;
            }
        }
        else {
            // TODO: check if log is correct

            computed_mutation_sloped = (1 - (i / individuals) * task->config_ga.mutation_param.mutation_slope) * computed_mutation;
            if (computed_mutation_sloped < 0) {
                computed_mutation_sloped = 0;
            }

            task->config_ga.mutation_param.mutation_rate[i] = computed_mutation_sloped * (task->config_ga.optimizer_param.max_mutations - task->config_ga.optimizer_param.min_mutations) + task->config_ga.optimizer_param.min_mutations;

            // now add the distribution according to mutation alpha and beta
            /*double sigmoid_factor = -1 + 2 / (1 + exp(-task->config_ga.mutation_param.mutation_alpha * computed_mutation * exp(i * task->config_ga.mutation_param.mutation_beta)));
            computed_mutation = sigmoid_factor * (task->config_ga.optimizer_param.max_mutations - task->config_ga.optimizer_param.min_mutations) + task->config_ga.optimizer_param.min_mutations;

            if (computed_mutation < task->config_ga.optimizer_param.min_mutations) {
                task->config_ga.mutation_param.mutation_rate[i] = task->config_ga.optimizer_param.min_mutations;
            }
            else if (computed_mutation > task->config_ga.optimizer_param.max_mutations) {
                task->config_ga.mutation_param.mutation_rate[i] = task->config_ga.optimizer_param.max_mutations;
            }
            else {
                task->config_ga.mutation_param.mutation_rate[i] = computed_mutation;
            }*/
        }
    }
}

//static void compute_flatten_factors(task_param_t* task, adaptive_memory_t* adaptive_memory, double best_result) {
//    // Based on dispersion of the group compared to the best result and the average result of the group
//    // calculate the flatten factor and bias
//
//
//}


void adapt_param(task_param_t* task, gene_pool_t* gene_pool, adaptive_memory_t* adaptive_memory) {
	
	double best_result = gene_pool->pop_result_set[gene_pool->sorted_indexes[gene_pool->individuals - 1]];

	if (adaptive_memory->iteration_counter == 0) {
        adaptive_memory->convergence_moving_window_alpha = 2 / ((double) task->config_ga.optimizer_param.convergence_moving_window_size+1);
        //adaptive_memory->convergence_moving_window_beta = 1 / (double) task->config_ga.optimizer_param.convergence_moving_window_size;
	}

    // Compute mutation rate
    compute_mutation_rate(task, adaptive_memory, best_result, gene_pool->individuals);

    // Check for convergence & runtime params
    check_convergence(task, adaptive_memory, best_result);

    // Compute flatten factor
}