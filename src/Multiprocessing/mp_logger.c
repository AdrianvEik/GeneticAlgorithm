
#include "mp_logger.h"
#include "mp_consts.h"
#include "mp_progress_disp.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../Helper/Struct.h"
#include "../Helper/rng.h"

void init_task_result_queue(task_result_queue_t* task_result_queue, runtime_param_t runtime_param, console_queue_t* console_queue) {
	task_result_queue->result_list = (task_result_t*)malloc(sizeof(task_result_t) * runtime_param.logging_param.queue_size);
	if (task_result_queue->result_list == NULL) {
		printf("Memory allocation failed: init_task_result_queue");
		exit(255);
	}

	task_result_queue->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if (task_result_queue->lock == NULL) {
		printf("Memory allocation failed: init_task_result_queue");
		exit(255);
	}

	//TODO: Should this be here?
    task_result_queue->console_queue = console_queue;

	task_result_queue->first_task_id = 0;
	task_result_queue->next_task_id = 0;
	task_result_queue->runtime_param = runtime_param;

	task_result_queue->bin_single_entry_length = sizeof(int) * 4 + sizeof(double) + sizeof(double) * runtime_param.genes * 3;
	if (runtime_param.logging_param.include_config == 1) {
		task_result_queue->bin_single_entry_length += sizeof(int) * runtime_param.logging_param.config_int_count + sizeof(double) * runtime_param.logging_param.config_double_count;
	}


	if (runtime_param.logging_param.write_csv == 1) {
		const len_of_engineering_double = 15; // -1.123456e+123;
		const len_of_formatted_int = 12; // -12345678901;
		task_result_queue->csv_single_entry_length = 4 * len_of_formatted_int + 1 * len_of_engineering_double + runtime_param.genes * len_of_engineering_double * 3;
		if (runtime_param.logging_param.include_config == 1) {
			task_result_queue->csv_single_entry_length += runtime_param.logging_param.config_int_count * len_of_formatted_int + runtime_param.logging_param.config_double_count * len_of_engineering_double;
		}

	}
	pthread_mutex_init(task_result_queue->lock, NULL);

	return task_result_queue;
}

void free_task_result_queue(task_result_queue_t* task_result_queue) {
	pthread_mutex_destroy(task_result_queue->lock);
    free(task_result_queue->result_list);
    free(task_result_queue->lock);
}

void stop_result_logger(task_result_queue_t* task_result_queue, int thread_count) {
	for (int i = 0; i < thread_count; i++) {
		task_result_t result;
		result.task_type = TERMINATE_THREAD;
		add_result(task_result_queue, &result);
	}

	pthread_join(task_result_queue->thread_id, NULL);
}


void init_task_result(task_result_queue_t* task_result_queue, task_result_t* task_result, int entry_count) {
	if (entry_count == 0) {
		return;
	}

	task_result->bin_buffer = malloc(sizeof(unsigned char) * task_result_queue->bin_single_entry_length * entry_count);
	if (task_result->bin_buffer == NULL) {
		printf("Memory allocation failed: init_task_result");
		exit(255);
	}

	if (task_result_queue->runtime_param.logging_param.write_csv == 1) {
		task_result->csv_buffer = malloc(sizeof(char) * task_result_queue->csv_single_entry_length * entry_count);
		if (task_result->csv_buffer == NULL) {
			printf("Memory allocation failed: init_task_result");
			exit(255);
		}
		task_result->csv_buffer[0] = '\0';
	}
	else {
		task_result->csv_buffer = NULL;
	}

	task_result->bin_position = 0;
	task_result->csv_position = 0;
	//DEBUG
	task_result->bin_single_entry_length = task_result_queue->bin_single_entry_length * entry_count;
	task_result->csv_single_entry_length = task_result_queue->csv_single_entry_length * entry_count;
}

void add_result(task_result_queue_t* task_result_queue, task_result_t* result) {
	int result_added = 0;
	while (!result_added) {
		pthread_mutex_lock(task_result_queue->lock);
		if (task_result_queue->first_task_id == (task_result_queue->next_task_id + 1) % task_result_queue->runtime_param.logging_param.queue_size) {
			pthread_mutex_unlock(task_result_queue->lock);
			Sleep(1000);
			continue;
		}
		task_result_queue->result_list[task_result_queue->next_task_id] = *result;
		task_result_queue->next_task_id = (task_result_queue->next_task_id + 1) % task_result_queue->runtime_param.logging_param.queue_size;
		result_added = 1;
		pthread_mutex_unlock(task_result_queue->lock);
	}
}

void get_result(task_result_queue_t* task_result_queue, task_result_t* result) {
	int result_retrieved = 0;
	while (!result_retrieved) {
		pthread_mutex_lock(task_result_queue->lock);
		if (task_result_queue->first_task_id == task_result_queue->next_task_id) {
			pthread_mutex_unlock(task_result_queue->lock);
			Sleep(1000);
			continue;
		}
		*result = task_result_queue->result_list[task_result_queue->first_task_id];
		task_result_queue->first_task_id = (task_result_queue->first_task_id + 1) % task_result_queue->runtime_param.logging_param.queue_size;
		pthread_mutex_unlock(task_result_queue->lock);
		result_retrieved = 1;
	}
}

void free_task_result(task_result_t* task_result) {
	if (task_result->csv_buffer != NULL) {
		free(task_result->csv_buffer);
	}
	free(task_result->bin_buffer);
}