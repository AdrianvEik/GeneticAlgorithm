
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

void init_task_result_queue(task_result_queue_t* task_result_queue, runtime_param_t runtime_param, int queue_size) {
	task_result_queue->result_list = (task_result_t*)malloc(sizeof(task_result_t) * queue_size);
	if (task_result_queue->result_list == NULL) {
		printf("Memory allocation failed: init_task_result_queue");
		exit(255);
	}

	task_result_queue->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if (task_result_queue->lock == NULL) {
		printf("Memory allocation failed: init_task_result_queue");
		exit(255);
	}

	task_result_queue->queue_size = queue_size;
	task_result_queue->first_task_id = 0;
	task_result_queue->next_task_id = 0;
    task_result_queue->runtime_param = runtime_param;

	pthread_mutex_init(task_result_queue->lock, NULL);

	return task_result_queue;
}

void free_task(task_param_t* task) {
    free(task->lower);
    free(task->upper);
    free(task->paramset);
}

void free_task_result(task_result_t* task_result) {
    free(task_result->lower);
    free(task_result->upper);
    free(task_result->paramset);
}



void add_result(task_result_queue_t* task_result_queue, task_result_t* result) {
    int result_added = 0;
    while (!result_added) {
        pthread_mutex_lock(task_result_queue->lock);
        if (task_result_queue->first_task_id == (task_result_queue->next_task_id + 1) % task_result_queue->queue_size) {
            pthread_mutex_unlock(task_result_queue->lock);
            Sleep(1000);
            continue;
        }
        task_result_queue->result_list[task_result_queue->next_task_id] = *result;
        task_result_queue->next_task_id = (task_result_queue->next_task_id + 1) % task_result_queue->queue_size;
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
        task_result_queue->first_task_id = (task_result_queue->first_task_id + 1) % task_result_queue->queue_size;
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
	task->task_type = 0;
	task->lower = (double*)malloc(sizeof(double) * runtime_param.genes);
	task->upper = (double*)malloc(sizeof(double) * runtime_param.genes);
	task->paramset = (double*)malloc(sizeof(double) * runtime_param.genes);
	if (task->lower == NULL || task->upper == NULL || task->paramset == NULL) {
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

			for (int j = 0; j < runtime_param.genes; j++) {
				task.lower[j] = config_ga.population_param.lower[j] + (config_ga.population_param.upper[j] - config_ga.population_param.lower[j]) / tasks_per_gene[j] * (position[j]);
				task.upper[j] = config_ga.population_param.upper[j] - (config_ga.population_param.upper[j] - config_ga.population_param.lower[j]) / tasks_per_gene[j] * (tasks_per_gene[j] - position[j] - 1);
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
        task.task_type = 1;
        add_task(task_queue, &task);
    }
	for (int j = 0; j < thread_count; j++) {
		pthread_join(task_queue->thread_id[j], NULL);
	}
}

void stop_result_logger(task_result_queue_t* task_result_queue, int thread_count) {
    for (int i = 0; i < thread_count; i++) {
        task_result_t result;
        result.task_type = 1;
        add_result(task_result_queue, &result);
    }

    pthread_join(task_result_queue->thread_id, NULL);
}