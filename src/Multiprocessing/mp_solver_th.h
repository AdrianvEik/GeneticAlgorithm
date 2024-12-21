
#ifndef _MP_SOLVER_TH_H_
#define _MP_SOLVER_TH_H_

#include <pthread.h>
#include "../Helper/Struct.h"
#include "mp_logger.h"


struct task_param_s {
	int task_type; // 0: GA, 1: kill
	int task_id;
	double* lower;
	double* upper;
	//double* paramset;
	config_ga_t config_ga;
};

typedef struct task_param_s task_param_t;


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
	runtime_param_t runtime_param;
};

typedef struct thread_param_s thread_param_t;

void init_task_queue(task_queue_t* task_queue, int queue_size, task_result_queue_t* task_result_queue, int thread_count);
void free_task_queue(task_queue_t* task_queue);
void init_task(runtime_param_t runtime_param, config_ga_t config_ga, task_param_t* task);
void add_task(task_queue_t* task_queue, task_param_t* task);
void get_task(task_queue_t* task_queue, task_param_t* task);
void stop_solver_threads(task_queue_t* task_queue, int thread_count);

#endif _MP_SOLVER_TH_H_



