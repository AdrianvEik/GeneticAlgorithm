
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#include "pthread.h"

#include "multiprocessing.h"

//pthread_mutex_t current_task_id_lock = PTHREAD_MUTEX_INITIALIZER;
//int current_task_id = 0;
//
//int get_task_id() {
//	pthread_mutex_lock(&current_task_id_lock);
//    int task_id = current_task_id;
//    current_task_id++;
//    pthread_mutex_unlock(&current_task_id_lock);
//    return task_id;
//}

//pthread_mutex_t current_thread_id_lock = PTHREAD_MUTEX_INITIALIZER;
//int current_thread_id = 0;
//
//int get_thread_id() {
//	pthread_mutex_lock(&current_thread_id_lock);
//	int thread_id = current_thread_id;
//	current_thread_id++;
//	pthread_mutex_unlock(&current_thread_id_lock);
//	return thread_id;
//}

void init_task_result_queue(task_result_queue_t* task_result_queue, runtime_param_t runtime_param) {
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
    task_result_queue->progress.kill_progress_display = 0;
    task_result_queue->progress.tasks_completed = 0;
    task_result_queue->progress.best_result = -INFINITY;

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

void free_task(task_param_t* task) {
    free(task->lower);
    free(task->upper);
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

void free_task_result(task_result_t* task_result) {
	if (task_result->csv_buffer != NULL) {
		free(task_result->csv_buffer);
	}
    free(task_result->bin_buffer);
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

void init_task_queue(task_queue_t* task_queue, int queue_size, task_result_queue_t* task_result_queue, int thread_count) {
    task_param_t* task_list = (task_param_t*)malloc(sizeof(task_param_t) * queue_size);
    if (task_list == NULL) {
        printf("Memory allocation failed: init_task_queue");
        exit(255);
    }
	pthread_t* thread_id;
	thread_id = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    if (thread_id == NULL) {
        printf("Memory allocation failed: init_task_queue");
        exit(255);
    }

    task_queue->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (task_queue->lock == NULL) {
        printf("Memory allocation failed: init_task_queue");
        exit(255);
    }

    task_queue->thread_id = thread_id;
	task_queue->task_list = task_list;
    task_queue->queue_size = queue_size;
    task_queue->task_result_queue = task_result_queue;
    task_queue->current_task_id = 0;
    task_queue->first_task_id = 0;
    task_queue->next_task_id = 0;
    pthread_mutex_init(task_queue->lock, NULL);

    return task_queue;
}

void free_task_queue(task_queue_t* task_queue) {
    pthread_mutex_destroy(task_queue->lock);
    free(task_queue->task_list);

	pthread_mutex_destroy(task_queue->task_result_queue->lock);
	free(task_queue->task_result_queue->result_list);
	free(task_queue->thread_id);
}

void add_task(task_queue_t* task_queue, task_param_t* task) {
	while (1) {
		pthread_mutex_lock(task_queue->lock);
		if (task_queue->first_task_id == (task_queue->next_task_id + 1) % task_queue->queue_size) {
			pthread_mutex_unlock(task_queue->lock);
			Sleep(1000);
			continue;
        }
		task->task_id = task_queue->current_task_id;
		task_queue->task_list[task_queue->next_task_id] = *task;
		task_queue->next_task_id = (task_queue->next_task_id + 1) % task_queue->queue_size;
        task_queue->current_task_id++;
		pthread_mutex_unlock(task_queue->lock);
        break;
	}
}

void get_task(task_queue_t* task_queue, task_param_t* task) {
    while (1) {
		pthread_mutex_lock(task_queue->lock);
        if (task_queue->first_task_id == task_queue->next_task_id) {
            pthread_mutex_unlock(task_queue->lock);
            Sleep(1000);
            continue;
        }
		*task = task_queue->task_list[task_queue->first_task_id];
		task_queue->first_task_id = (task_queue->first_task_id + 1) % task_queue->queue_size;
		pthread_mutex_unlock(task_queue->lock);
        break;
	}
}

void new_task(runtime_param_t runtime_param, config_ga_t config_ga, task_param_t* task) {
	task->task_type = GA_TASK;
	task->lower = (double*)malloc(sizeof(double) * runtime_param.genes);
	task->upper = (double*)malloc(sizeof(double) * runtime_param.genes);
	if (task->lower == NULL || task->upper == NULL) {
		printf("Memory allocation failed: new_task");
		exit(255);
	}
    task->config_ga = config_ga;
    return task;
}

//void generate_task(task_param_t* task_list, int* task_id, runtime_param_t runtime_param, config_ga_t config_ga, int current_gene, int* tasks_per_gene, int* position) {
void generate_task(task_queue_t* task_queue, runtime_param_t runtime_param, config_ga_t config_ga, int current_gene, int* tasks_per_gene, int* position) {

	for (int i = 0; i < tasks_per_gene[current_gene]; i++) {
		position[current_gene] = i;

		if (current_gene != runtime_param.genes - 1) {
			generate_task(task_queue, runtime_param, config_ga, current_gene + 1, tasks_per_gene, position);
		}
		else {
			task_param_t task;
			new_task(runtime_param, config_ga, &task);
			if (runtime_param.zone_enable == 1) {
				for (int j = 0; j < runtime_param.genes; j++) {
					task.lower[j] = config_ga.population_param.lower[j] + (config_ga.population_param.upper[j] - config_ga.population_param.lower[j]) / tasks_per_gene[j] * (position[j]);
					task.upper[j] = config_ga.population_param.upper[j] - (config_ga.population_param.upper[j] - config_ga.population_param.lower[j]) / tasks_per_gene[j] * (tasks_per_gene[j] - position[j] - 1);
				}
			}
            else {
                for (int j = 0; j < runtime_param.genes; j++) {
                    task.lower[j] = config_ga.population_param.lower[j];
                    task.upper[j] = config_ga.population_param.upper[j];
                }
            }
            add_task(task_queue, &task);
		}
	}

}

void make_task_list(runtime_param_t* runtime_param, config_ga_t config_ga, task_queue_t* task_queue) {
	int task_id = 0;
	int task_count = runtime_param->task_count; // power of 2
	int remaining_tasks = task_count;

	int generated_task_count = 1;
	int minimum_tasks_per_gene = 2;

	int* tasks_per_gene = (int*)malloc(sizeof(int) * runtime_param->genes);

	if (tasks_per_gene == NULL) {
		printf("Memory allocation failed: make_task_list");
		exit(255);
	}

	for (int i = 0; i < runtime_param->genes; i++) {
		if (remaining_tasks == 1) {
			tasks_per_gene[i] = 1;
		}
		else {
			if (remaining_tasks <= minimum_tasks_per_gene * (runtime_param->genes - i)) {
				tasks_per_gene[i] = minimum_tasks_per_gene;
			}
			else {
				tasks_per_gene[i] = (int) nearbyint(pow(remaining_tasks, 1.0 / (runtime_param->genes - i)));

			}
		}
		remaining_tasks /= tasks_per_gene[i];
		generated_task_count *= tasks_per_gene[i];
		printf("Tasks per gene %d: %d\n", i, tasks_per_gene[i]);
	}
	

	runtime_param->task_count = generated_task_count;

	int* position = (int*)malloc(sizeof(int) * runtime_param->genes);
	if (position == NULL) {
		printf("Memory allocation failed: make_task_list");
		exit(255);
	}


	generate_task(task_queue, *runtime_param, config_ga, 0, tasks_per_gene, position);

	free(position);
	free(tasks_per_gene);
}

void stop_threads(task_queue_t* task_queue, int thread_count) {
    for (int i = 0; i < thread_count; i++) {
        task_param_t task;
        task.task_type = TERMINATE_THREAD;
        add_task(task_queue, &task);
    }
	for (int j = 0; j < thread_count; j++) {
		pthread_join(task_queue->thread_id[j], NULL);
	}
}

void stop_result_logger(task_result_queue_t* task_result_queue, int thread_count) {
    for (int i = 0; i < thread_count; i++) {
        task_result_t result;
        result.task_type = TERMINATE_THREAD;
        add_result(task_result_queue, &result);
    }

    pthread_join(task_result_queue->thread_id, NULL);
    pthread_join(task_result_queue->thread_id_progress_display, NULL);
}