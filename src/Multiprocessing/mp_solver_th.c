
#include "mp_solver_th.h"



void init_task_queue(task_queue_t* task_queue, uint32_t queue_size, task_result_queue_t* task_result_queue, uint32_t thread_count) {
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
}

void free_task_queue(task_queue_t* task_queue) {
    pthread_mutex_destroy(task_queue->lock);
    free(task_queue->task_list);

    free(task_queue->thread_id);
}

void init_task(runtime_param_t runtime_param, config_ga_t config_ga, task_param_t* task) {
    task->task_type = GA_TASK;
    task->lower = malloc(sizeof(double) * runtime_param.genes);
    task->upper = malloc(sizeof(double) * runtime_param.genes);
    if (task->lower == NULL || task->upper == NULL) EXIT_MEM_ERROR();
    task->zone_mask = malloc(runtime_param.genes * sizeof(int));
    task->zone_id = malloc(runtime_param.genes * sizeof(int));
    if (task->zone_mask == NULL || task->zone_id == NULL) EXIT_MEM_ERROR();

    task->config_ga = config_ga;

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
    free(task->zone_mask);
    free(task->zone_id);   
}

void stop_task_solver_threads(task_queue_t* task_queue, uint32_t thread_count) {
    for (uint32_t i = 0; i < thread_count; i++) {
        task_param_t task;
        task.task_type = TERMINATE_THREAD;
        add_task(task_queue, &task);
    }
    for (uint32_t j = 0; j < thread_count; j++) {
        pthread_join(task_queue->thread_id[j], NULL);
    }
}

void init_fx_task_queue(fx_task_queue_t* fx_task_queue, uint32_t queue_size, uint32_t thread_count, uint32_t task_size_fx) {
    fx_task_param_t* fx_task_list = (fx_task_param_t*)malloc(sizeof(fx_task_param_t) * queue_size);
    if (fx_task_list == NULL) EXIT_MEM_ERROR();

    pthread_t* thread_id;
    thread_id = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    if (thread_id == NULL) EXIT_MEM_ERROR();

    fx_task_queue->lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (fx_task_queue->lock == NULL) EXIT_MEM_ERROR();

    fx_task_queue->thread_id = thread_id;
    fx_task_queue->fx_task_list = fx_task_list;
    fx_task_queue->queue_size = queue_size;
    fx_task_queue->first_task_id = 0;
    fx_task_queue->next_task_id = 0;
    fx_task_queue->task_size_fx = task_size_fx;
    pthread_mutex_init(fx_task_queue->lock, NULL);
}

void free_fx_task_queue(fx_task_queue_t* fx_task_queue) {
    pthread_mutex_destroy(fx_task_queue->lock);
    free(fx_task_queue->fx_task_list);

    free(fx_task_queue->thread_id);
}

void add_fx_task(fx_task_queue_t* fx_task_queue, fx_task_param_t fx_task) {
    while (1) {
        pthread_mutex_lock(fx_task_queue->lock);
        if (fx_task_queue->first_task_id == (fx_task_queue->next_task_id + 1) % fx_task_queue->queue_size) {
            pthread_mutex_unlock(fx_task_queue->lock);
            Sleep(1000);
            continue;
        }
        fx_task_queue->fx_task_list[fx_task_queue->next_task_id] = fx_task;
        fx_task_queue->next_task_id = (fx_task_queue->next_task_id + 1) % fx_task_queue->queue_size;
        pthread_mutex_unlock(fx_task_queue->lock);
        break;
    }
}

void get_fx_task(fx_task_queue_t* fx_task_queue, fx_task_param_t* fx_task) {
    while (1) {
        pthread_mutex_lock(fx_task_queue->lock);
        if (fx_task_queue->first_task_id == fx_task_queue->next_task_id) {
            pthread_mutex_unlock(fx_task_queue->lock);
            Sleep(1000);
            continue;
        }
        *fx_task = fx_task_queue->fx_task_list[fx_task_queue->first_task_id];
        fx_task_queue->first_task_id = (fx_task_queue->first_task_id + 1) % fx_task_queue->queue_size;
        pthread_mutex_unlock(fx_task_queue->lock);
        break;
    }
}

void stop_fx_task_threads(fx_task_queue_t* task_queue, uint32_t thread_count) {
    for (uint32_t i = 0; i < thread_count; i++) {
        fx_task_param_t task;
        task.task_type = TERMINATE_THREAD;
        add_fx_task(task_queue, task);
    }
    for (uint32_t j = 0; j < thread_count; j++) {
        pthread_join(task_queue->thread_id[j], NULL);
    }
}