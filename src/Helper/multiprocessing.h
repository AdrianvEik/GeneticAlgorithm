#include <pthread.h>
#include <stdio.h>

#include "../Helper/Struct.h"
#include "../Helper/rng.h"

#ifndef MULTIPROCESSING_H
#define MULTIPROCESSING_H

struct task_param_s {
	int task_type; // 0: GA, 1: kill
    int task_id;
	double* lower;
	double* upper;
	double* paramset;
    struct config_ga_s config_ga;
};

typedef struct task_param_s task_param_t;

struct task_result_s {
    int task_type; // 0: GA, 1: kill
    int iterations;
    int task_id;
	double* lower;
    double* upper;
    double* paramset;
	double result;
};

typedef struct task_result_s task_result_t;

struct task_result_queue_s {
    int queue_size;
    task_result_t* result_list;
	pthread_t thread_id;
	FILE* fileptr;
	FILE* fileptrcsv;
	FILE* fileptrconfig;
	runtime_param_t runtime_param;
	int first_task_id;
    int next_task_id;
    pthread_mutex_t* lock;
};

typedef struct task_result_queue_s task_result_queue_t;

struct task_queue_s {
	int queue_size;
    int current_task_id;
	pthread_t* thread_id;
    task_result_queue_t* task_result_queue;
	task_param_t* task_list;
	int first_task_id;
	int next_task_id;
	pthread_mutex_t* lock;
};

typedef struct task_queue_s task_queue_t;

struct thread_param_s {
	int status;
	task_queue_t* task_queue;
	struct runtime_param_s runtime_param;
};

typedef struct thread_param_s thread_param_t;


void make_task_list(runtime_param_t* runtime_param, config_ga_t config_ga, task_queue_t* task_queue);

void free_task(task_param_t* task);
void free_task_result(task_result_t* task_result);


void init_task_result_queue(task_result_queue_t* task_result_queue,runtime_param_t runtime_param,int queue_size);
void add_result(task_result_queue_t* task_result_queue, task_result_t* result);
void get_result(task_result_queue_t* task_result_queue, task_result_t* result);
void stop_result_logger(task_result_queue_t* task_result_queue, int thread_count);

void init_task_queue(task_queue_t* task_queue,int queue_size, task_result_queue_t* task_result_queue, int thread_count);
void free_task_queue(task_queue_t* task_queue);
void add_task(task_queue_t* task_queue, task_param_t* task);
void get_task(task_queue_t* task_queue, task_param_t* task);
void stop_threads(task_queue_t* task_queue, int thread_count);

#endif // MULTIPROCESSING_H