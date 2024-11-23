
#include <math.h>

#include "../Optimisation/Optimizer.h"

void new_adaptive_memory(adaptive_memory_t* adaptive_memory) {
    adaptive_memory->iteration_counter = 0;
    adaptive_memory->convergence_counter = 0;
    adaptive_memory->convergence_reached = 0;
    adaptive_memory->convergence_moving_window = 0;
    adaptive_memory->previous_best_result = 0;
}

void adapt_param(task_param_t* task, gene_pool_t* gene_pool, adaptive_memory_t* adaptive_memory) {
	
	double best_result = gene_pool->pop_result_set[gene_pool->sorted_indexes[gene_pool->individuals - 1]];
    double computed_mutation;

	// Check for convergence & runtime params
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


	if (adaptive_memory->iteration_counter == 0) {
        adaptive_memory->convergence_moving_window_alpha = (task->config_ga.optimizer_param.convergence_moving_window_size-1)/ task->config_ga.optimizer_param.convergence_moving_window_size;
        adaptive_memory->convergence_moving_window_beta = 1 / task->config_ga.optimizer_param.convergence_moving_window_size;
	}

    adaptive_memory->convergence_moving_window = adaptive_memory->convergence_moving_window_alpha * adaptive_memory->convergence_moving_window + adaptive_memory->convergence_moving_window_beta * best_result;
	
    if (adaptive_memory->convergence_moving_window == 0) {
        if (task->config_ga.mutation_param.mutation_rate < task->config_ga.optimizer_param.max_mutations){
            task->config_ga.mutation_param.mutation_rate++;
        }
    }
    else {
        // TODO: check if log is correct
        computed_mutation = log(task->config_ga.optimizer_param.convergence_threshold / adaptive_memory->convergence_moving_window) * task->config_ga.optimizer_param.mutation_factor;
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