

#include "Function.h"



static double Styblinski_Tang_fx(double* parameter_set, uint32_t genes) {
	double result = 0;
	for (uint32_t i = 0; i < genes; i++) {
		result += (pow(parameter_set[i], 4)) - (16 * pow(parameter_set[i], 2)) + (5 * parameter_set[i]);
	}
	return result / 2;
}

// def wheelers_ridge(x: Union[np.ndarray, list], a: float = 1.5) -> float:
//     """
//     Compute the Wheelersridge function for given x1 and x2

//     :param x: list with x1 (otype: float) and x2 (otype: float)
//     :param a: additional parameter typically a=1.5

//     :return: Value f(x1, x2, a), real float
//     """
//     x1, x2 = x
//     return -np.exp(-(x1 * x2 - a) ** 2 - (x2 - a) ** 2)

static double wheelers_ridge_fx(double* parameter_set, uint32_t genes) {
	double a = 1.5;

	// check if genes = 2
    if (genes != 2) EXIT_WITH_ERROR("Genes must be 2 for Wheelers Ridge", 255);

	double x1 = parameter_set[0];
	double x2 = parameter_set[1];
	return -1 * exp(-1 * pow(x1 * x2 - a, 2) - pow(x2 - a, 2));
}

double weigh_result(double result, progress_t progress) {
	double weighed_result = 0;

	if (result < 1000) {
		weighed_result = 0;
		return weighed_result;
	}
    double max_result = 32 * 39.166169; // Styblinski Tang max for 32 genes
    weighed_result += (1 - ((max_result - result) / max_result)) * 0.1;
    weighed_result += (1 / progress.elapsed_time) * 0.8;
    weighed_result += (1 / progress.best_result_iteration) * 0.1;
	printf(
        "Weighed result: %f (result: %f, elapsed time %f, iteration %d)\n", 
		weighed_result,
        result,
		progress.elapsed_time,
		progress.best_result_iteration
	);
	return weighed_result;
}

static inline uint16_t scaler(uint16_t min, uint16_t max, uint16_t param) {
	return min + param / (UINT16_MAX / (max - min));
}

double Genetic_Algorithm(config_ga_t config_ga, runtime_param_t runtime_param, progress_t* end_result);

static double optimize_fx_ga(uint32_t* paramset, uint32_t parent_individual, uint32_t parent_iteration) {
	runtime_param_t runtime_param = default_runtime_param();
	runtime_param.zone_enable = 0;
	runtime_param.task_count_solver = 1;
	runtime_param.individuals = 32;
	runtime_param.genes = 32;
	runtime_param.thread_count_solver = 1;
	runtime_param.elitism = 3;
	runtime_param.thread_count_fx = 1;
	runtime_param.task_size_fx = 0;
	runtime_param.random_seed = 0xAbAe;

	runtime_param.logging_param.include_config = 0;
	//runtime_param.logging_param.write_config = 1; // JSON dump
	runtime_param.logging_param.write_csv = 1;
	runtime_param.logging_param.write_bin = 0;
	runtime_param.logging_param.export_interval = 10;
	runtime_param.logging_param.top_n_export = 1;
	runtime_param.logging_param.console_enabled = 0;
	runtime_param.logging_param.fully_qualified_basename = malloc(255 * sizeof(char));
	snprintf(runtime_param.logging_param.fully_qualified_basename, 255, "C:/temp/GA/GA_%d_%d\0", parent_iteration, parent_individual);


	config_ga_t config_ga = default_config(runtime_param);
	config_ga.selection_param.selection_method = selection_method_roulette;
	config_ga.population_param.reseed_bottom_N = 1;
	config_ga.crossover_param.crossover_method = crossover_method_two_point;

	uint16_t* paramset_u16 = (uint16_t*)paramset;
	// int 1000 < x < 50000
	config_ga.optimizer_param.max_iterations = scaler(1000, 10000, paramset_u16[0]);
	// int 100 < x < 10000
	config_ga.optimizer_param.convergence_window = scaler(100, config_ga.optimizer_param.max_iterations, paramset_u16[1]);
	// int 10 < x < 1000
	config_ga.optimizer_param.convergence_moving_window_size = scaler(10, config_ga.optimizer_param.convergence_window, paramset_u16[2]);
	// int 10 < x < 1000
	config_ga.optimizer_param.max_mutations = scaler(10, 1000, paramset_u16[3]);
	// int 1 < x < 100
	config_ga.optimizer_param.min_mutations = scaler(1, 100, paramset_u16[4]);
	// double 0.0 < x < 1.0
	config_ga.mutation_param.mutation_slope = (double)paramset_u16[5] / (double)UINT16_MAX;

	config_ga.fx_param.fx_method = fx_method_Styblinski_Tang;

	for (uint32_t i = 0; i < runtime_param.genes; i++) {
		config_ga.population_param.lower[i] = -5.0;
		config_ga.population_param.upper[i] = 5.0;
	}
	progress_t progress;
	double result = Genetic_Algorithm(config_ga, runtime_param, &progress);

	free_config_ga(&config_ga);
    free(runtime_param.logging_param.fully_qualified_basename);
	return weigh_result(result, progress);
}

void process_fx_set(gene_pool_t* gene_pool, task_param_t* task, uint32_t individual_min, uint32_t individual_max) {
	/*

	:param pop: matrix of individuals as double (individuals x genes)
	:param individuals: number of individuals
	:param genes: number of genes
	:param fx: fitness function (double array x0 x1 ... xn)
	:param result: matrix of fitness values (individuals x 1)

	*/
	for (uint32_t individual = individual_min; individual <= individual_max; individual++) {
		uint32_t i = gene_pool->sorted_indexes[individual];

		// convert the gene pool bin to double
		if (task->config_ga.fx_param.fx_data_type == fx_data_type_double) {
			for (uint32_t j = 0; j < gene_pool->genes; j++) {
				gene_pool->pop_param_double[i][j] = (double)(gene_pool->pop_param_bin[i][j] * (task->upper[j] - task->lower[j])) / UINT32_MAX + task->lower[j];
			}
		}
		else if (task->config_ga.fx_param.fx_data_type == fx_data_type_int) {
			for (uint32_t j = 0; j < gene_pool->genes; j++) {
				gene_pool->pop_param_bin[i][j] = (gene_pool->pop_param_bin[i][j] & task->zone_mask[j]) | task->zone_id[j];
			}
		}

		if (task->config_ga.fx_param.fx_method == fx_method_Styblinski_Tang) {
			task->config_ga.fx_param.fx_optim_mode = fx_optim_mode_minimize;
			gene_pool->pop_result_set[i] = task->config_ga.fx_param.fx_optim_mode * Styblinski_Tang_fx(gene_pool->pop_param_double[i], gene_pool->genes);

		}
		else if (task->config_ga.fx_param.fx_method == fx_method_Genetic_Algorithm) {
			task->config_ga.fx_param.fx_optim_mode = fx_optim_mode_maximize;
			gene_pool->pop_result_set[i] = task->config_ga.fx_param.fx_optim_mode * optimize_fx_ga(gene_pool->pop_param_bin[i], i, gene_pool->iteration_number);
		}


		else if (task->config_ga.fx_param.fx_method == fx_method_pointer) {
			if (task->config_ga.fx_param.fx_function == NULL) {
				EXIT_WITH_ERROR("Function pointer is NULL", 255);
			}

			void** param_ptr_array = NULL;

			if (task->config_ga.fx_param.fx_data_type == fx_data_type_double) {
				param_ptr_array = (void**)gene_pool->pop_param_double;
			}
			else if (task->config_ga.fx_param.fx_data_type == fx_data_type_int) {
				param_ptr_array = (void**)gene_pool->pop_param_bin;
			}
			else {
				EXIT_WITH_ERROR("Unknown data type", 255);
			}

			if (param_ptr_array == NULL) {
				EXIT_WITH_ERROR("Param ptr is NULL", 255);
			}

			gene_pool->pop_result_set[i] = task->config_ga.fx_param.fx_optim_mode *
				task->config_ga.fx_param.fx_function(param_ptr_array[i], gene_pool->genes);

		}
		else {
			EXIT_WITH_ERROR("Unkown fitness function", 255);
		}

        gene_pool->fx_ready[individual] = 1;
	}
}

static void wait_fx_tasks_finish(gene_pool_t* gene_pool) {
	while (1) {
		int all_done = 1;
		for (uint32_t i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
			if (gene_pool->fx_ready[i] == 0) {
				all_done = 0;
				break;
			}
		}
		if (all_done) {
			break;
		}
		Sleep(10);
    }
}


void process_fx(gene_pool_t* gene_pool, task_param_t* task, fx_task_queue_t* fx_task_queue) {

	if (fx_task_queue->task_size_fx == 0) {
		for (uint32_t i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
			process_fx_set(gene_pool, task, i, i);
		}
	}
	else {
		uint32_t i = 0;

		while (i < gene_pool->individuals - gene_pool->elitism) {
			fx_task_param_t fx_task;
			fx_task.gene_pool = gene_pool;
			fx_task.individual_min = i;
			fx_task.individual_max = i + fx_task_queue->task_size_fx - 1;
			if (fx_task.individual_max > (gene_pool->individuals - gene_pool->elitism)) {
				fx_task.individual_max = gene_pool->individuals - gene_pool->elitism;
			}
			fx_task.task_param = task;
			fx_task.task_type = FX_TASK;

            gene_pool->fx_ready[i] = 0;

			add_fx_task(fx_task_queue, fx_task);
			i += fx_task_queue->task_size_fx;
		}

        wait_fx_tasks_finish(gene_pool);
	}
}
