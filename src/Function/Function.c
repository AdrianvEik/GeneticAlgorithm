

#include "Function.h"

double Styblinski_Tang_fx(double* parameter_set, int genes) {
	double result = 0;
	for (int i = 0; i < genes; i++) {
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

double wheelers_ridge_fx(double* parameter_set, int genes) {
	double a = 1.5;

	// check if genes = 2
    if (genes != 2) EXIT_WITH_ERROR("Genes must be 2 for Wheelers Ridge", 255);

	double x1 = parameter_set[0];
	double x2 = parameter_set[1];
	return -1 * exp(-1 * pow(x1 * x2 - a, 2) - pow(x2 - a, 2));
}

void process_fx_set(gene_pool_t* gene_pool, task_param_t* task, int individual_min, int individual_max) {
	/*

	:param pop: matrix of individuals as double (individuals x genes)
	:param individuals: number of individuals
	:param genes: number of genes
	:param fx: fitness function (double array x0 x1 ... xn)
	:param result: matrix of fitness values (individuals x 1)

	*/
	for (int individual = individual_min; individual <= individual_max; individual++) {
		int i = gene_pool->sorted_indexes[individual];

		// convert the gene pool bin to double
		if (task->config_ga.fx_param.fx_data_type == fx_data_type_double) {
			for (int j = 0; j < gene_pool->genes; j++) {
				gene_pool->pop_param_double[i][j] = (double)(gene_pool->pop_param_bin[i][j] * (task->upper[j] - task->lower[j])) / UINT32_MAX + task->lower[j];
			}
		}
		else if (task->config_ga.fx_param.fx_data_type == fx_data_type_int) {
			for (int j = 0; j < gene_pool->genes; j++) {
				gene_pool->pop_param_bin[i][j] = (gene_pool->pop_param_bin[i][j] & task->zone_mask[j]) | task->zone_id[j];
			}
		}

		if (task->config_ga.fx_param.fx_method == fx_method_Styblinski_Tang) {
			task->config_ga.fx_param.fx_optim_mode = -1;
			gene_pool->pop_result_set[i] = task->config_ga.fx_param.fx_optim_mode * Styblinski_Tang_fx(gene_pool->pop_param_double[i], gene_pool->genes);

		}
		else if (task->config_ga.fx_param.fx_method == fx_method_Wheelers_Ridge) {
			task->config_ga.fx_param.fx_optim_mode = -1;
			gene_pool->pop_result_set[i] = task->config_ga.fx_param.fx_optim_mode * wheelers_ridge_fx(gene_pool->pop_param_double[i], gene_pool->genes);
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
	}
}

void process_fx(gene_pool_t* gene_pool, task_param_t* task, fx_task_queue_t* fx_task_queue) {

	if (fx_task_queue->task_size_fx == 0) {
		for (int i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
			process_fx_set(gene_pool, task, i, i);
		}
	}
	else {
		int i = 0;

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

			add_fx_task(fx_task_queue, fx_task);
			i += fx_task_queue->task_size_fx;
		}
	}
}
