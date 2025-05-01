#include "process.h"

void indexed_bubble_sort(double* arr, int* indices, int size) {
	int swapped = 1;
	int temp_idx;

	for (int i = 0; i < size && swapped; i++) {
		swapped = 0;
		for (int j = 0; j < size - i - 1; j++) {
			if (arr[indices[j]] > arr[indices[j + 1]]) {
				temp_idx = indices[j];
				indices[j] = indices[j + 1];
				indices[j + 1] = temp_idx;
				swapped = 1;
			}
		}
	}
}

static void post_process_population(gene_pool_t* gene_pool, population_param_t* pop_param) {
	int unique = 1;
	// eliminate duplicates
	for (int i = pop_param->reseed_bottom_N; i < gene_pool->individuals - 1; i++) {
		unique = 1;
		if (gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] == gene_pool->pop_result_set[gene_pool->sorted_indexes[i + 1]]) {
			for (int k = 0; k < gene_pool->genes; k++) {
				if (gene_pool->pop_param_bin[gene_pool->sorted_indexes[i]][k] == gene_pool->pop_param_bin[gene_pool->sorted_indexes[i + 1]][k]) {
					unique = 0;
					break;
				}
			}

			if (unique == 0) {
				fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
			}
		}
	}
    // reseed bottom N
    for (int i = 0; i < pop_param->reseed_bottom_N; i++) {
		fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
    }
}

void process_pop(gene_pool_t* gene_pool, task_param_t* task) {
	// TODO: check individual even nr 
	// TODO: refractor individuals and genes to _count

	process_fx(gene_pool, &(task->config_ga.fx_param), task->lower, task->upper); // pop, individuals, genes -> ?

	// worst-best scaling according to fitness and fit function (lin, exp, log, sig, norm)
	process_flatten(gene_pool, &(task->config_ga.flatten_param));

	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->sorted_indexes[i] = i;
	}

	indexed_bubble_sort(gene_pool->flatten_result_set, gene_pool->sorted_indexes, gene_pool->individuals);

	// copy sorted to selected
	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->selected_indexes[i] = gene_pool->sorted_indexes[i];
	}

	process_selection(gene_pool, &(task->config_ga.selection_param));

	// crossover
	process_crossover(gene_pool, &(task->config_ga.crossover_param));

	// mutation
	process_mutation(gene_pool, &(task->config_ga.mutation_param));

    // Eliminate duplicates and reseed bottom N
	post_process_population(gene_pool, &(task->config_ga.population_param));
}