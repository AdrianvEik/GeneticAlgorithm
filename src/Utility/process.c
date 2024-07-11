#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/multiprocessing.h"
#include "flatten.h"
#include "../Function/Function.h"
#include "selection.h"
#include "crossover.h"

#include "pop.h"

#include "process.h"
#include "mutation.h"

void eliminate_duplicates(gene_pool_t* gene_pool) {
	int unique = 1;
	// eliminate duplicates
	for (int i = 0; i < gene_pool->individuals - 1; i++) {
		unique = 1;
		if (gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] == gene_pool->pop_result_set[gene_pool->sorted_indexes[i + 1]]) {
			for (int k = 0; k < gene_pool->genes; k++) {
				if (gene_pool->pop_param_double[gene_pool->sorted_indexes[i]][k] == gene_pool->pop_param_double[gene_pool->sorted_indexes[i + 1]][k]) {
					unique = 0;
					break;
				}
			}

			if (unique == 0) {
				fill_individual(gene_pool, gene_pool->sorted_indexes[i]);
			}
		}
	}
}

void process_pop(gene_pool_t* gene_pool, config_ga_t* config_ga, task_param_t* task) {
	// TODO: check individual even nr 
	// TODO: refractor individuals and genes to _count

	process_fx(gene_pool, &(config_ga->fx_param), task->lower, task->upper); // pop, individuals, genes -> ?

	process_flatten(gene_pool, &(config_ga->flatten_param));

	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->sorted_indexes[i] = i;
	}

	indexed_bubble_sort(gene_pool->flatten_result_set, gene_pool->sorted_indexes, gene_pool->individuals);

	// copy sorted to selected
	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->selected_indexes[i] = gene_pool->sorted_indexes[i];
	}

	process_selection(gene_pool, &(config_ga->selection_param));

	// // crossover
	process_crossover(gene_pool, &(config_ga->crossover_param));

	// mutation
	mutate32(gene_pool, &(config_ga->mutation_param));

	eliminate_duplicates(gene_pool);
}