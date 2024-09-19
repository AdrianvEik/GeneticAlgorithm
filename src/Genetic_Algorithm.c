
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include <windows.h>
#include <pthread.h>

#include "Genetic_Algorithm.h"

#include "Utility/process.h"
#include "Utility/pop.h"
#include "Utility/crossover.h"
#include "Utility/mutation.h"
#include "Utility/selection.h"
#include "Utility/flatten.h"

#include "Function/Function.h"

#include "Helper/Helper.h"
#include "Helper/Struct.h"
#include "Helper/multiprocessing.h"

#include "Logger/logging.h"

pthread_mutex_t current_task_id_lock = PTHREAD_MUTEX_INITIALIZER;
int current_task_id = 0;

int get_task_id() {
	pthread_mutex_lock(&current_task_id_lock);
    int task_id = current_task_id;
    current_task_id++;
    pthread_mutex_unlock(&current_task_id_lock);
    return task_id;
}

void process_task(thread_param_t* thread_param) {
	printf("Thread %d, Task %d\n", thread_param->thread_id, thread_param->task_id);
	thread_param->status = 1; // In progress

	gene_pool_t gene_pool;
	//printf("cfgbin2int, %f", thread_param->config_ga.fx_param.lower[0]);
	//printf("cfgtoursize, %d", thread_param->config_ga.selection_param.selection_tournament_size);

	gene_pool.genes = thread_param->runtime_param.genes;
	gene_pool.individuals = thread_param->runtime_param.individuals;
	gene_pool.elitism = thread_param->runtime_param.elitism;

	init_gene_pool(&gene_pool);

	fill_pop(&gene_pool);

	double previous_best_res = 0.0f;
	double best_res = 0.0f;
	int convergence_counter = 0;

	char* log_file;
	log_file = (char*)malloc(sizeof(char) * 255);

	if (thread_param->runtime_param.fully_qualified_basename == NULL) {
		thread_param->runtime_param.fully_qualified_basename = "C:/temp/GA\0";
	}

	strcpy_s(log_file, 255, thread_param->runtime_param.fully_qualified_basename);
	strcat_s(log_file, 255, "Thread%d\0");
	sprintf_s(log_file, 255, log_file, thread_param->task_id);

	open_file(gene_pool, thread_param, log_file);
	write_config(gene_pool, *thread_param);
	
	for (int i = 0; i < thread_param->runtime_param.max_iterations; i++) {
		// Process Population
		process_pop(&gene_pool, &thread_param->config_ga, &(thread_param->task_list[thread_param->task_id]));

		write_param(gene_pool, *thread_param, i);

		best_res = gene_pool.pop_result_set[gene_pool.sorted_indexes[gene_pool.individuals - 1]];

		// Check for convergence & runtime params

		if (fabs(best_res - previous_best_res) < thread_param->runtime_param.convergence_threshold) {
			convergence_counter++;
			if (convergence_counter > thread_param->runtime_param.convergence_window) {
				printf("Converged at iteration: %d\n", i);
				break;
			}
		}
		else {
			convergence_counter = 0;
		}

		if (best_res - previous_best_res > 0.0f) {
//			printf("Iteration: %d Gain: %0.3f best res: %0.3f (idx = %d), 2nd best res: %0.3f (idx = %d) 3rd best res %0.3f (idx = %d)\n", i,
//				(best_res - previous_best_res),
//				gene_pool.pop_result_set[gene_pool.sorted_indexes[gene_pool.individuals - 1]], gene_pool.sorted_indexes[gene_pool.individuals - 1],
//				gene_pool.pop_result_set[gene_pool.sorted_indexes[gene_pool.individuals - 2]], gene_pool.sorted_indexes[gene_pool.individuals - 2],
//				gene_pool.pop_result_set[gene_pool.sorted_indexes[gene_pool.individuals - 3]], gene_pool.sorted_indexes[gene_pool.individuals - 3]);
		}

		previous_best_res = best_res;
	}

	thread_param->task_list[thread_param->task_id].result = best_res;
	printf("Thread %d, Task %d Param: ", thread_param->thread_id, thread_param->task_id);

	for (int i = 0; i < thread_param->runtime_param.genes; i++) {
		thread_param->task_list[thread_param->task_id].paramset[i] = gene_pool.pop_param_double[gene_pool.sorted_indexes[gene_pool.individuals - 1]][i];
		printf("%f ", thread_param->task_list[thread_param->task_id].paramset[i]);

	}
	printf("Result %f\n", thread_param->task_list[thread_param->task_id].result);

	//TODO: sort results, return best result

	close_file(*thread_param);

	// Free pop_parameter_bin
	free_gene_pool(&gene_pool);

	thread_param->status = 2; // Completed
}

void* process_thread(thread_param_t* thread_param) {
	int task_id = get_task_id();
	while (task_id < thread_param->runtime_param.task_count) {
		thread_param->task_id = task_id;
		process_task(thread_param);
		task_id = get_task_id();
	}
	return NULL;
}

void start_threads(task_param_t* task_list, runtime_param_t runtime_param, config_ga_t config_ga) {
	const parallel = 1;

	if (parallel == 0) {
		thread_param_t thread_param;
		thread_param.task_list = task_list;
		thread_param.runtime_param = runtime_param;
		thread_param.config_ga = config_ga;

		double best_result = 0.0f;
		for (int i = 0; i < runtime_param.task_count; i++) {
			thread_param.thread_id = 0;
			thread_param.task_id = i;
			process_task(&thread_param);

			if (TRUE) {
				for (int j = 0; j <= i; j++) {
					if (task_list[j].result > best_result) {
						best_result = task_list[j].result;
					}
				}
				printf("Best result: %f\n", best_result);
			}

		}
	}
	else {
		int NTHREADS = 8;

		pthread_t *thread_id;

		thread_id = (pthread_t *) malloc(sizeof(pthread_t) * NTHREADS);

		int i, j;
		thread_param_t *thread_param;

		thread_param = (thread_param_t *) malloc(sizeof(thread_param_t) * NTHREADS);

		if (thread_param == NULL) {
            printf("Memory allocation failed");
            exit(255);
        }

		for (i = 0; i < NTHREADS; i++)
		{
			int retid = 0;
			thread_param[i].task_list = task_list;
			thread_param[i].runtime_param = runtime_param;
			thread_param[i].config_ga = config_ga;

			thread_param[i].thread_id = i;
			//thread_param[i].task_id = i;
			retid = pthread_create(&thread_id[i], NULL, (void *) process_thread, (void *) & thread_param[i]);

			if(retid)
            {
                printf("Error creating thread %d\n", i);
                exit(1);
            }
		}

		for (j = 0; j < NTHREADS; j++)
		{
			pthread_join(thread_id[j], NULL);
		}

		pthread_mutex_destroy(&current_task_id_lock);
		free(thread_param);
		free(thread_id);

	}
}


void Genetic_Algorithm(config_ga_t config_ga, runtime_param_t runtime_param) {
	gene_pool_t gene_pool;
	//	printf("cfgbin2int, %f", config_ga.fx_param.lower[0]);
	//	printf("cfgtoursize, %d", config_ga.selection_param.selection_tournament_size);

	gene_pool.genes = runtime_param.genes;
	gene_pool.individuals = runtime_param.individuals;
	gene_pool.elitism = runtime_param.elitism;

	init_gene_pool(&gene_pool);

	fill_pop(&gene_pool);

	double previous_best_res = 0.0f;
	double best_res = 0.0f;
	int convergence_counter = 0;

	// print max iter
	printf("Max Iterations: %d\n", runtime_param.max_iterations);

	task_param_t* task_list = make_task_list(&runtime_param, config_ga);

	start_threads(task_list, runtime_param, config_ga);

	free_task_list(task_list, runtime_param);

	// Free pop_parameter_bin
	free_gene_pool(&gene_pool);
}

int main() {
	int repeats = 1;
	for (int i = 0; i < repeats; i++) {
		printf("\n Run number: %d\n", i);
		runtime_param_t runtime_param;
		runtime_param.max_iterations = 10000;
		runtime_param.convergence_threshold = 1e-8;
		runtime_param.convergence_window = 1000;
		runtime_param.individuals = 32;
		runtime_param.genes = 4;
		runtime_param.elitism = 2;
		runtime_param.fully_qualified_basename = "C:/temp/GA\0";
		runtime_param.task_count = 64;

		flatten_param_t flatten_param;
		flatten_param.flatten_method = 0;
		flatten_param.flatten_factor = 1.0f;
		flatten_param.flatten_bias = 0.0f;
		flatten_param.flatten_optim_mode = flatten_method_none;

		crossover_param_t crossover_param;
		crossover_param.crossover_method = crossover_method_uniform32;
		crossover_param.crossover_prob = 0.5f;

		mutation_param_t mutation_param;
		mutation_param.mutation_method = 0;
		mutation_param.mutation_prob = 0.5f;
		mutation_param.mutation_rate = 6;

		fx_param_t fx_param;
		fx_param.fx_method = fx_method_Styblinski_Tang;
		fx_param.lower = malloc(sizeof(double) * 3);
		fx_param.upper = malloc(sizeof(double) * 3);
		for (int i = 0; i < runtime_param.genes; i++) {
			fx_param.lower[i] = -5.0f;
            fx_param.upper[i] = 5.0f;
		}

		selection_param_t selection_param;
		selection_param.selection_method = 0;
		selection_param.selection_div_param = 0.0f;
		selection_param.selection_prob_param = 0.0f;
		selection_param.selection_temp_param = 10.0f;
		selection_param.selection_tournament_size = 0;

		config_ga_t config_ga;
		config_ga.selection_param = selection_param;
		config_ga.flatten_param = flatten_param;
		config_ga.crossover_param = crossover_param;
		config_ga.mutation_param = mutation_param;
		config_ga.fx_param = fx_param;


		//strcpy_s(runtime_param.fully_qualified_basename, 255, "C:/temp/GA\0");
		//printf("%s\n", runtime_param.fully_qualified_basename);
		//printf("%d\n", strlen(runtime_param.fully_qualified_basename));
		//Genetic_Algorithm(config_ga, runtime_param);


		task_param_t* task_list = make_task_list(&runtime_param, config_ga);

		start_threads(task_list, runtime_param, config_ga);

		free_task_list(task_list, runtime_param);
	}
	return 0;
}