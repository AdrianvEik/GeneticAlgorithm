#include "process.h"


//void indexed_merge_sort(double* arr, int* indices, int* temp_workspace, int size) {
//    merge(arr, indices, temp_workspace, 0, size, size);
//}

static uint32_t dedupe_population(gene_pool_t* gene_pool) {
	int unique = 1;
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__m512i);
	uint32_t duplicate_count = 0;

	__mAVXi** pop_param_bin_ptr = (__mAVXi**)gene_pool->pop_param_bin;

	// eliminate duplicates
	for (uint32_t i = 0; i < gene_pool->individuals - 1; i++) {
		unique = 0;
		if (gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] == gene_pool->pop_result_set[gene_pool->sorted_indexes[i + 1]]) {
			for (int j = 0; j < memory_blocks; j++) {
				uint8_t mask = _mm512_cmpeq_epu64_mask(pop_param_bin_ptr[gene_pool->sorted_indexes[i]][j], pop_param_bin_ptr[gene_pool->sorted_indexes[i + 1]][j]);
				if (mask != 0xFFFFFFFFFFFFFFFF) {
					unique = 1;
					break;
				}
			}

			if (unique == 0) {
                gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] = nextafter(gene_pool->pop_result_set[gene_pool->sorted_indexes[i]], -DBL_MAX); // mark duplicate fitness for reseeding
                duplicate_count++;
			}
		}
	}
	return duplicate_count;
}

static void reseed_population(gene_pool_t* gene_pool, population_param_t* pop_param, uint32_t duplicate_count) {
    duplicate_count = (duplicate_count > pop_param->reseed_bottom_N) ? duplicate_count : pop_param->reseed_bottom_N;
	for (uint32_t i = 0; i < duplicate_count; i++) {
		fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
	}
}

static void post_process_population(gene_pool_t* gene_pool, population_param_t* pop_param) {
	int unique = 1;
    int reseed_remaining = pop_param->reseed_bottom_N;
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__m512i);

	__mAVXi** pop_param_bin_ptr = (__mAVXi**)gene_pool->pop_param_bin;

	// eliminate duplicates
	for (uint32_t i = 0; i < gene_pool->individuals - 1; i++) {
		unique = 0;
		if (gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] == gene_pool->pop_result_set[gene_pool->sorted_indexes[i + 1]]) {
			for (int j = 0; j < memory_blocks; j++) {
				uint8_t mask = _mm512_cmpeq_epu64_mask(pop_param_bin_ptr[gene_pool->sorted_indexes[i]][j], pop_param_bin_ptr[gene_pool->sorted_indexes[i + 1]][j]);
				if (mask != 0xFFFFFFFFFFFFFFFF) {
					unique = 1;
					break;
				}
			}

			if (unique == 0) {
				fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
				reseed_remaining--;
			}
		}
	}
    // reseed bottom N
    for (uint32_t i = 0; i < reseed_remaining; i++) {
		fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
    }
}

void process_pop(gene_pool_t* gene_pool, task_param_t* task, fx_task_queue_t* fx_task_queue) {
	// TODO: check individual even nr 
	// TODO: refractor individuals and genes to _count
	uint32_t dupe_count = 0;

	process_fx(gene_pool, task, fx_task_queue); // pop, individuals, genes -> ?

	//indexed_merge_sort(gene_pool->flatten_result_set, gene_pool->sorted_indexes, gene_pool->sorted_indexes_temp, 0, gene_pool->individuals, gene_pool->individuals);
    memcpy_s(gene_pool->sorted_indexes, gene_pool->individuals * sizeof(uint32_t), pop_index_sequence, gene_pool->individuals * sizeof(uint32_t));
	indexed_bitonic_sort_8v(gene_pool->pop_result_set, gene_pool->sorted_indexes, gene_pool->sorted_indexes_temp, gene_pool->individuals);

	dupe_count = dedupe_population(gene_pool);
	memcpy_s(gene_pool->sorted_indexes, gene_pool->individuals * sizeof(uint32_t), pop_index_sequence, gene_pool->individuals * sizeof(uint32_t));

	indexed_bitonic_sort_8v(gene_pool->pop_result_set, gene_pool->sorted_indexes, gene_pool->sorted_indexes_temp, gene_pool->individuals);

	// worst-best scaling according to fitness and fit function (lin, exp, log, sig, norm)
	process_flatten(gene_pool, &(task->config_ga.flatten_param));

	// copy sorted to selected
	for (uint32_t i = 0; i < gene_pool->individuals; i++) {
		gene_pool->selected_indexes[i] = gene_pool->sorted_indexes[i];
	}

	process_selection(gene_pool, &(task->config_ga.selection_param));

	// crossover
	process_crossover(gene_pool, &(task->config_ga.crossover_param));

	// mutation
	process_mutation(gene_pool, &(task->config_ga.mutation_param));

    // Eliminate duplicates and reseed bottom N
    reseed_population(gene_pool, &(task->config_ga.population_param), dupe_count);
	//post_process_population(gene_pool, &(task->config_ga.population_param));
}