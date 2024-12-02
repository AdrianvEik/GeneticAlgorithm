
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include <windows.h>
#include <pthread.h>

#include "Genetic_Algorithm.h"


void report_task(task_queue_t* task_queue, task_param_t* task, adaptive_memory_t* adaptive_memory, thread_param_t* thread_param, gene_pool_t* gene_pool) {

	for (int individual = 0; individual < thread_param->runtime_param.logging_param.top_n_export; individual++) {
		task_result_t task_result;
		task_result.task_type = 0;
		task_result.iteration = adaptive_memory->iteration_counter;
		task_result.task_id = task->task_id;
		task_result.individual_id = gene_pool->sorted_indexes[gene_pool->individuals - individual - 1];
		task_result.position = individual;
        task_result.converged = adaptive_memory->convergence_reached;
		task_result.result = gene_pool->pop_result_set[task_result.individual_id];
		task_result.lower = malloc(sizeof(double) * thread_param->runtime_param.genes);
		task_result.upper = malloc(sizeof(double) * thread_param->runtime_param.genes);
		task_result.paramset = malloc(sizeof(double) * thread_param->runtime_param.genes);
		if (task_result.lower == NULL || task_result.upper == NULL || task_result.paramset == NULL) {
			printf("Memory allocation failed");
			exit(255);
		}

		for (int i = 0; i < thread_param->runtime_param.genes; i++) {
			task_result.lower[i] = task->lower[i];
			task_result.upper[i] = task->upper[i];
			task_result.paramset[i] = gene_pool->pop_param_double[task_result.individual_id][i];
		}

		if (thread_param->runtime_param.logging_param.include_config) {
			// config reports:
			// (int) mutation_rate
			// (double) computed_mutation
			// (double) convergence_moving_window

			task_result.config_int = malloc(sizeof(int) * 1);
			task_result.config_double = malloc(sizeof(double) * 3);
			if (task_result.config_int == NULL || task_result.config_double == NULL) {
				printf("Memory allocation failed");
				exit(255);
			}

			task_result.config_int[0] = task->config_ga.mutation_param.mutation_rate;
			task_result.config_double[0] = adaptive_memory->computed_mutation;
			task_result.config_double[1] = adaptive_memory->convergence_moving_window;
		}

		task_queue_t* task_queue = thread_param->task_queue;
		task_result_queue_t* task_result_queue = task_queue->task_result_queue;
		add_result(task_result_queue, &task_result);
	}
}

void process_task(thread_param_t* thread_param, task_param_t* task, gene_pool_t* gene_pool) {
	//printf("Thread %d, Task %d\n", thread_param->thread_id, thread_param->task_id);

	fill_pop(gene_pool, task->config_ga.population_param);

    adaptive_memory_t adaptive_memory;
    new_adaptive_memory(&adaptive_memory);

	while (1) {

		// Process Population
		process_pop(gene_pool, task);

		adapt_param(task, gene_pool, &adaptive_memory);

		if (adaptive_memory.convergence_reached == 1 ||
			(thread_param->runtime_param.logging_param.export_interval != 0 &&
				adaptive_memory.iteration_counter % thread_param->runtime_param.logging_param.export_interval == 0)
			) {
			report_task(thread_param->task_queue, task, &adaptive_memory, thread_param, gene_pool);
		}

        if (adaptive_memory.convergence_reached == 1) {
            break;
        }

	}
    free_task(task);

	thread_param->status = 2; // Completed
}


void* process_log_thread(task_result_queue_t* task_result_queue) {
	task_result_t task_result;

	char* log_file;
	log_file = (char*)malloc(sizeof(char) * 255);

    if (log_file == NULL) {
        printf("Memory allocation failed: process_log_thread");
        exit(255);
    }

	if (task_result_queue->runtime_param.logging_param.fully_qualified_basename == NULL) {
		strcpy_s(log_file, 255, "C:/temp/GA\0");

	}
	else {
		strcpy_s(log_file, 255, task_result_queue->runtime_param.logging_param.fully_qualified_basename);
	}


	open_file(task_result_queue);

    double current_best_res = 0.0f;

	// Save the best gene_pool
	task_result_t best_result;

	//task_result.task_id = task->task_id;
	//task_result.iterations = iterations_required;
	//task_result.result = best_res;
	best_result.paramset = malloc(sizeof(double) * task_result_queue->runtime_param.genes);
	best_result.lower = malloc(sizeof(double) * task_result_queue->runtime_param.genes);
	best_result.upper = malloc(sizeof(double) * task_result_queue->runtime_param.genes);

	if (best_result.lower == NULL || best_result.upper == NULL || best_result.paramset == NULL) {
		printf("Memory allocation failed: process_log_thread");
		exit(255);
	}

	if (task_result_queue->runtime_param.logging_param.include_config == 1) {
		best_result.config_int = malloc(sizeof(int) * task_result_queue->runtime_param.logging_param.config_int_count);
		best_result.config_double = malloc(sizeof(double) * task_result_queue->runtime_param.logging_param.config_double_count);
		if (best_result.config_int == NULL || best_result.config_double == NULL) {
			printf("Memory allocation failed: process_log_thread");
			exit(255);
		}
	}

    while (1) {
        get_result(task_result_queue, &task_result);
        if (task_result.task_type == 1) {
			write_param(*task_result_queue, best_result);
            free_task_result(&best_result);
            break;
        }

		if (task_result.converged == 1 && task_result.result > current_best_res) {
			current_best_res = task_result.result;
			best_result.iteration = task_result.iteration;
			best_result.task_id = task_result.task_id;
            best_result.individual_id = task_result.individual_id;
            best_result.position = task_result.position;
			best_result.result = task_result.result;
			for (int i = 0; i < task_result_queue->runtime_param.genes; i++) {
				best_result.lower[i] = task_result.lower[i];
				best_result.upper[i] = task_result.upper[i];
				best_result.paramset[i] = task_result.paramset[i];
			}
            if (task_result_queue->runtime_param.logging_param.include_config) {
                for (int i = 0; i < task_result_queue->runtime_param.logging_param.config_int_count; i++) {
                    best_result.config_int[i] = task_result.config_int[i];
                }
                for (int i = 0; i < task_result_queue->runtime_param.logging_param.config_double_count; i++) {
                    best_result.config_double[i] = task_result.config_double[i];
                }
            }  
		}

        write_param(*task_result_queue, task_result);

        if (task_result_queue->runtime_param.logging_param.include_config) {
            free(task_result.config_int);
            free(task_result.config_double);
        }
        free_task_result(&task_result);
    }

}

void* process_task_thread(thread_param_t* thread_param) {
	seedRandThread(NULL); // todo fix thread local storage

	gene_pool_t gene_pool;
	//printf("cfgbin2int, %f", thread_param->config_ga.fx_param.lower[0]);
	//printf("cfgtoursize, %d", thread_param->config_ga.selection_param.selection_tournament_size);

	gene_pool.genes = thread_param->runtime_param.genes;
	gene_pool.individuals = thread_param->runtime_param.individuals;
	gene_pool.elitism = thread_param->runtime_param.elitism;

	init_gene_pool(&gene_pool);
    init_pre_compute(&gene_pool);

	task_param_t task;
	while (1) {
		get_task(thread_param->task_queue, &task);
        if (task.task_type == 1) {
            break;
        }
		process_task(thread_param, &task, &gene_pool);
	}

	free_pre_compute();
	free_gene_pool(&gene_pool);
	return NULL;
}

void start_threads(task_queue_t* task_queue, runtime_param_t runtime_param, config_ga_t config_ga, thread_param_t* thread_param) {
	const parallel = 1;


	if (parallel == 0) {
		//init_thread_rng(1);

		//thread_param_t thread_param;
		//thread_param.task_queue = task_queue;
		//thread_param.runtime_param = runtime_param;
		//thread_param.config_ga = config_ga;

		//seedRandThread(0, NULL);

		//double best_result = 0.0f;
		//process_thread(&thread_param);

	}
	else {
		int NTHREADS = runtime_param.thread_count;

		int i, j;

		thread_param = (thread_param_t *) malloc(sizeof(thread_param_t) * NTHREADS);

		if (thread_param == NULL) {
            printf("Memory allocation failed: start_threads");
            exit(255);
        }

		int retid = 0;
		retid = pthread_create(&(task_queue->task_result_queue->thread_id), NULL, (void*)process_log_thread, (void*)task_queue->task_result_queue);

        if (retid) {
            printf("Error creating thread\n");
            exit(1);
        }

		for (i = 0; i < NTHREADS; i++)
		{
            thread_param[i].task_queue = malloc(sizeof(task_queue_t));
            if (thread_param[i].task_queue == NULL) {
                printf("Memory allocation failed: start_threads");
                exit(255);
            }

			thread_param[i].task_queue = task_queue;
			thread_param[i].runtime_param = runtime_param;
			
			//thread_param[i].task_id = i;
			retid = pthread_create(&(task_queue->thread_id[i]), NULL, (void *) process_task_thread, (void *) & thread_param[i]);

			if(retid)
            {
                printf("Error creating thread %d\n", i);
                exit(1);
            }
		}
	}
}

logging_param_t default_logging_param() {
    // Setups default logging parameters
    logging_param_t logging_param;

    logging_param.fully_qualified_basename = "C:/temp/GA\0";
    logging_param.top_n_export = 1;
    logging_param.export_interval = 100;
    logging_param.include_config = 1;
    logging_param.write_csv = 1;
    logging_param.config_int_count = 1;
    logging_param.config_double_count = 2;
    return logging_param;
}

runtime_param_t default_runtime_param() {
    // Setups default runtime parameters
    runtime_param_t runtime_param;

    runtime_param.individuals = 16;
    runtime_param.genes = 4;
    runtime_param.elitism = 2;
    runtime_param.task_count = 81;
	runtime_param.thread_count = 16;
    runtime_param.logging_param = default_logging_param();

    return runtime_param;
}

config_ga_t default_config(runtime_param_t runtime_param) {
    // Setups default configuration
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
	fx_param.fx_optim_mode = 1;
	fx_param.fx_function = NULL;

	population_param_t pop_param;
	pop_param.sampling_type = pop_normal;
	pop_param.sigma = 1;
	pop_param.lower = malloc(sizeof(double) * runtime_param.genes);
	pop_param.upper = malloc(sizeof(double) * runtime_param.genes);
	for (int i = 0; i < runtime_param.genes; i++) {
		pop_param.lower[i] = -5.0f;
		pop_param.upper[i] = 5.0f;
	}

	selection_param_t selection_param;
	selection_param.selection_method = selection_method_roulette;
	selection_param.selection_div_param = 0.5f;
	selection_param.selection_prob_param = 0.2f;
	selection_param.selection_temp_param = 10.0f;
	selection_param.selection_tournament_size = 4;

    optimizer_param_t optimizer_param;
    optimizer_param.convergence_moving_window_size = 10;
    optimizer_param.min_mutations = 1;
    optimizer_param.max_mutations = 100;
    optimizer_param.mutation_factor = 1e3;
    optimizer_param.max_iterations = 1000;
    optimizer_param.convergence_threshold = 1e-8;
    optimizer_param.convergence_window = 100;

	config_ga_t config_ga;
	config_ga.selection_param = selection_param;
	config_ga.flatten_param = flatten_param;
	config_ga.crossover_param = crossover_param;
	config_ga.mutation_param = mutation_param;
	config_ga.fx_param = fx_param;
	config_ga.population_param = pop_param;
    config_ga.optimizer_param = optimizer_param;

    return config_ga;
}

double Genetic_Algorithm(config_ga_t config_ga, runtime_param_t runtime_param) {

	double previous_best_res = 0.0f;
	double best_res = 0.0f;
	int convergence_counter = 0;
	task_result_queue_t task_result_queue;
	init_task_result_queue(&task_result_queue, runtime_param, runtime_param.thread_count * 4);
	task_queue_t task_queue;
	init_task_queue(&task_queue, runtime_param.thread_count * 4, &task_result_queue, runtime_param.thread_count);
	thread_param_t* thread_param;

	start_threads(&task_queue, runtime_param, config_ga, &thread_param);

	make_task_list(&runtime_param, config_ga, &task_queue);
	
	stop_threads(&task_queue, runtime_param.thread_count);
    stop_result_logger(&task_result_queue, runtime_param.thread_count); // todo add thread_join
	close_file(&task_result_queue);
	//free(thread_param); // todo: why?
	free_task_queue(&task_queue);
}

void free_config_ga(config_ga_t* config_ga) {
    free(config_ga->population_param.lower);
    free(config_ga->population_param.upper);
}

int main() {
	int repeats = 1;
	runtime_param_t runtime_param = default_runtime_param();
	config_ga_t config_ga = default_config(runtime_param);

	for (int i = 0; i < repeats; i++) {
		printf("\n Run number: %d\n", i);

		//strcpy_s(runtime_param.fully_qualified_basename, 255, "C:/temp/GA\0");
		//printf("%s\n", runtime_param.fully_qualified_basename);
		//printf("%d\n", strlen(runtime_param.fully_qualified_basename));
		//Genetic_Algorithm(config_ga, runtime_param);

        Genetic_Algorithm(config_ga, runtime_param);
	}
    free_config_ga(&config_ga);

	return 0;
}