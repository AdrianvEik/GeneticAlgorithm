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
#include "Helper/rng.h"
#include "Helper/error_handling.h"

#include "Multiprocessing/mp_logger.h"
#include "Multiprocessing/mp_solver_th.h"
#include "Multiprocessing/mp_task_gen.h"
#include "Multiprocessing/mp_progress_disp.h"
#include "Multiprocessing/mp_consts.h"
#include "Multiprocessing/mp_thread_locals.h"

#include "Logger/logging.h"
#include "Logger/progress_display.h"
#include "Logger/dump_config.h"

#include "Optimisation/Optimizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <Windows.h>
#include <pthread.h>

static void process_task(thread_param_t* thread_param, task_param_t* task, gene_pool_t* gene_pool) {
	fill_pop(gene_pool, task->config_ga.population_param, task->config_ga.fx_param);

    adaptive_memory_t adaptive_memory;
    new_adaptive_memory(&adaptive_memory);

	while (1) {

		// Process Population
		process_pop(gene_pool, task, thread_param->fx_task_queue);

		adapt_param(task, gene_pool, &adaptive_memory);

		if (adaptive_memory.convergence_reached == 1 ||
			(thread_param->runtime_param.logging_param.export_interval != 0 &&
				adaptive_memory.iteration_counter % thread_param->runtime_param.logging_param.export_interval == 0)
			) {
			if (adaptive_memory.convergence_reached == 1) {
				report_task(thread_param->task_queue, task, &adaptive_memory, thread_param, gene_pool, 1);
                con_printf(thread_param->task_queue->task_result_queue->console_queue, "Convergence reached at iteration %d\n", adaptive_memory.iteration_counter);
				break;
			}
			else {
				report_task(thread_param->task_queue, task, &adaptive_memory, thread_param, gene_pool, 0);

			}
		}
	}
    free_task(task);

	thread_param->status = 2; // Completed
}

static void process_log_thread(task_result_queue_t* task_result_queue) {
	task_result_t task_result;

	char* log_file;
	log_file = (char*)malloc(sizeof(char) * 255);

	if (log_file == NULL) EXIT_MEM_ERROR(); 
	

	if (task_result_queue->runtime_param.logging_param.fully_qualified_basename == NULL) {
		strcpy_s(log_file, 255, "C:/temp/GA\0");

	}
	else {
		strcpy_s(log_file, 255, task_result_queue->runtime_param.logging_param.fully_qualified_basename);
	}


	open_file(task_result_queue);

    double current_best_res = -INFINITY;

	// Save the best gene_pool
	task_result_t best_result;
	init_task_result(task_result_queue, &best_result, 1);

	clock_t start, current;
	start = clock();

	console_message_t print_str;

	//printf("\n\n\n\n\n\n"); // set the cursor below the progress, TODO: make nice 

	int last_message = 0;
	//task_result.task_id = task->task_id;
	//task_result.iterations = iterations_required;
	//task_result.result = best_res;
    int done = 0;
    while (!done) {
        while(get_result(task_result_queue, &task_result)) {
			if (task_result.task_type == TERMINATE_THREAD) {
				write_file_buffer(task_result_queue, &best_result);

				current = clock(); 
				task_result_queue->progress.elapsed_time = (double)(current - start) / CLOCKS_PER_SEC;

				// update the progress one last time
				display_progress(
					&task_result_queue->progress,
					task_result_queue->console_queue->message_list_size,
					task_result_queue->runtime_param.logging_param.console_enabled
				);
				
				free_task_result(&best_result);
                done = 1;
				break;
			}

			if (task_result.task_type == BEST_RESULT_TASK) {
				task_result_queue->progress.tasks_completed++;
				// add the results to be processed to an answer in the console
				// to be divided by the number of tasks completed
				task_result_queue->progress.average_result += task_result.result;
				// to be divided by the number of tasks completed - 1 and sqrt
				task_result_queue->progress.result_standard_deviation += pow(
					(task_result.result - (task_result_queue->progress.average_result) / task_result_queue->progress.tasks_completed), 2
				);

				if (task_result.result > current_best_res) {
					current_best_res = task_result.result;
					task_result_queue->progress.best_result = current_best_res;
					copy_task_result(&best_result, &task_result);
				}
			}
			write_file_buffer(task_result_queue, &task_result);

			free_task_result(&task_result);
		}
		
		current = clock(); // Update every second
		task_result_queue->progress.elapsed_time = (double)(current - start) / CLOCKS_PER_SEC;

		while (get_from_console_queue(task_result_queue->console_queue, &print_str)) {

			display_console_message(
				print_str.str,
				last_message,
				task_result_queue->runtime_param.logging_param.console_enabled
			);
			last_message = (last_message + 1) % task_result_queue->console_queue->message_list_size;

		}

		display_progress(
			&task_result_queue->progress,
			task_result_queue->console_queue->message_list_size,
			task_result_queue->runtime_param.logging_param.console_enabled
		);
		Sleep(500);
    }
	free(log_file);
}

static void process_task_thread(thread_param_t* thread_param) {
	seed_rand_threadlocal(thread_param->runtime_param.random_seed); // todo fix thread local storage

	gene_pool_t gene_pool;

	init_gene_pool(&gene_pool, &(thread_param->runtime_param));
    init_pre_compute(&gene_pool);

	task_param_t task;
	while (1) {
		get_task(thread_param->task_queue, &task);
        if (task.task_type == TERMINATE_THREAD) {
            break;
        }
		process_task(thread_param, &task, &gene_pool);
	}

	free_pre_compute();
	free_gene_pool(&gene_pool);
	return;
}

static void process_fx_task_thread(fx_task_queue_t* fx_task_queue) {
	fx_task_param_t fx_task;
	while (1) {
		get_fx_task(fx_task_queue, &fx_task);
		if (fx_task.task_type == TERMINATE_THREAD) {
			break;
		}

		process_fx_set(fx_task.gene_pool, fx_task.task_param, fx_task.individual_min, fx_task.individual_max);
	}
}

static void start_threads(task_queue_t* task_queue, fx_task_queue_t* fx_task_queue, runtime_param_t runtime_param, config_ga_t config_ga, thread_param_t* thread_param) {

	thread_param = (thread_param_t *) malloc(sizeof(thread_param_t) * runtime_param.thread_count_solver);

	if (thread_param == NULL) EXIT_MEM_ERROR();

	int retid = 0;
	retid = pthread_create(&(task_queue->task_result_queue->thread_id), NULL, (void*)process_log_thread, (void*)task_queue->task_result_queue);
    if (retid) EXIT_WITH_ERROR("Thread creation", 1);

	for (uint32_t i = 0; i < runtime_param.thread_count_fx; i++) {
		retid = pthread_create(&(fx_task_queue->thread_id[i]), NULL, (void*)process_fx_task_thread, (void*)fx_task_queue);
		if (retid) EXIT_WITH_ERROR("Thread creation", 1);
	}

	for (uint32_t i = 0; i < runtime_param.thread_count_solver; i++)
	{
		thread_param[i].task_queue = task_queue;
		thread_param[i].runtime_param = runtime_param;
		thread_param[i].fx_task_queue = fx_task_queue;
			
		//thread_param[i].task_id = i;
		retid = pthread_create(&(task_queue->thread_id[i]), NULL, (void *) process_task_thread, (void *) & thread_param[i]);

		if(retid) EXIT_WITH_ERROR("Thread creation", 1);
	}
}

double Genetic_Algorithm(config_ga_t config_ga, runtime_param_t runtime_param, progress_t* end_result) {
	verify_input_parameters(config_ga, runtime_param);
    if (runtime_param.logging_param.write_config == 1) {
		write_config(runtime_param, config_ga);
    }

	//double previous_best_res = -INFINITY;
	double best_res = -INFINITY;
	//int convergence_counter = 0;
	console_queue_t* console_queue = NULL;

	console_queue = init_console_queue();
	console_queue->message_count = runtime_param.zone_enable ? compute_task_count(&runtime_param) : runtime_param.task_count_solver;
	

	task_result_queue_t task_result_queue;
	init_task_result_queue(&task_result_queue, runtime_param, console_queue, config_ga.fx_param);

	task_queue_t task_queue;
	init_task_queue(&task_queue, runtime_param.thread_count_solver * 4, &task_result_queue, runtime_param.thread_count_solver);
	
	fx_task_queue_t fx_task_queue;
	init_fx_task_queue(&fx_task_queue, runtime_param.thread_count_fx * 4, runtime_param.thread_count_fx, runtime_param.task_size_fx);
	
	thread_param_t thread_param;

	start_threads(&task_queue, &fx_task_queue, runtime_param, config_ga, &thread_param);

	make_task_list(&runtime_param, config_ga, &task_queue);

	stop_task_solver_threads(&task_queue, runtime_param.thread_count_solver);

	stop_fx_task_threads(&fx_task_queue, runtime_param.thread_count_fx);

    stop_result_logger(&task_result_queue, runtime_param.thread_count_solver, &best_res);

	if (end_result != NULL){
		memcpy_s(end_result, sizeof(progress_t), &task_result_queue.progress, sizeof(progress_t));
	}

	close_file(&task_result_queue);
	free_task_queue(&task_queue);
	free_fx_task_queue(&fx_task_queue);
	free_console_queue(console_queue);

    return best_res;
}

static void free_config_ga(config_ga_t* config_ga) {
    free(config_ga->mutation_param.mutation_rate);
    free(config_ga->population_param.lower);
    free(config_ga->population_param.upper);
}

static inline uint16_t scaler(uint16_t min, uint16_t max, uint16_t param){
    return min + param / (UINT16_MAX / (max - min));
}

static double optimize_fx_ga(uint32_t* paramset, uint32_t n_params) {
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
	runtime_param.logging_param.write_csv = 0;
	runtime_param.logging_param.write_bin = 0;
	runtime_param.logging_param.export_interval = 10;
	runtime_param.logging_param.top_n_export = 1;
	runtime_param.logging_param.console_enabled = 0;


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
    config_ga.mutation_param.mutation_slope = (double)paramset_u16[5] / (double) UINT16_MAX;

	config_ga.fx_param.fx_method = fx_method_Styblinski_Tang;

	for (uint32_t i = 0; i < n_params; i++) {
		config_ga.population_param.lower[i] = -5.0;
		config_ga.population_param.upper[i] = 5.0;
	}

	double result = Genetic_Algorithm(config_ga, runtime_param, NULL);
	free_config_ga(&config_ga);
    return result;
}

int main() {
	uint32_t repeats = 1;
	runtime_param_t runtime_param = default_runtime_param();
	runtime_param.zone_enable = 1;
	runtime_param.task_count_solver = 1;
	runtime_param.individuals = 32;
	runtime_param.genes = 3;
	runtime_param.thread_count_solver = 1;
	runtime_param.elitism = 3;


	runtime_param.logging_param.include_config = 1;
    //runtime_param.logging_param.write_config = 1; // JSON dump
    runtime_param.logging_param.write_csv = 1;
    runtime_param.logging_param.write_bin = 0;
    runtime_param.logging_param.export_interval = 1;
	runtime_param.logging_param.top_n_export = 32;
	runtime_param.logging_param.console_enabled = 1;
	runtime_param.task_size_fx = 1;
	runtime_param.thread_count_fx = 8;

	
	config_ga_t config_ga = default_config(runtime_param);
	config_ga.selection_param.selection_method = selection_method_roulette;
	config_ga.population_param.reseed_bottom_N = 0;
    config_ga.crossover_param.crossover_method = crossover_method_two_point;

	config_ga.optimizer_param.convergence_window = 5;
	config_ga.optimizer_param.convergence_moving_window_size = 3;
	config_ga.optimizer_param.max_iterations = 20;
	config_ga.optimizer_param.max_mutations = 10;
    config_ga.optimizer_param.min_mutations = 1;
	config_ga.mutation_param.mutation_slope = 1.0;

    config_ga.fx_param.fx_method = fx_method_pointer;
    config_ga.fx_param.fx_function = &optimize_fx_ga;
	config_ga.fx_param.fx_data_type = fx_data_type_int;
    config_ga.fx_param.fx_optim_mode = fx_optim_mode_maximize;

    //config_ga.mutation_param.mutation_alpha = 10;
    //config_ga.mutation_param.mutation_beta = 0.1;

	for (uint32_t i = 0; i < repeats; i++) {
        Genetic_Algorithm(config_ga, runtime_param, NULL);
	}
    free_config_ga(&config_ga);

	return 0;
}