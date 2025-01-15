
#include "mp_solver_th.h"
#include "mp_progress_disp.h"
#include "mp_consts.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"


void init_task_queue(task_queue_t* task_queue, int queue_size, task_result_queue_t* task_result_queue, int thread_count) {
    task_param_t* task_list = (task_param_t*)malloc(sizeof(task_param_t) * queue_size);
    if (task_list == NULL) EXIT_MEM_ERROR();

    pthread_t* thread_id;
    thread_id = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    if (thread_id == NULL) EXIT_MEM_ERROR();

    task_queue->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (task_queue->lock == NULL) EXIT_MEM_ERROR();

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

    free(task_queue->thread_id);
}

void init_task(runtime_param_t runtime_param, config_ga_t config_ga, task_param_t* task) {
    task->task_type = GA_TASK;
    task->lower = (double*)malloc(sizeof(double) * runtime_param.genes);
    task->upper = (double*)malloc(sizeof(double) * runtime_param.genes);
    if (task->lower == NULL || task->upper == NULL) EXIT_MEM_ERROR();

    task->config_ga = config_ga;
    return task;
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

void free_task(task_param_t* task) {
    free(task->lower);
    free(task->upper);
}

void stop_solver_threads(task_queue_t* task_queue, int thread_count) {
    for (int i = 0; i < thread_count; i++) {
        task_param_t task;
        task.task_type = TERMINATE_THREAD;
        add_task(task_queue, &task);
    }
    for (int j = 0; j < thread_count; j++) {
        pthread_join(task_queue->thread_id[j], NULL);
    }
}