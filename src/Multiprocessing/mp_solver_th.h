
#ifndef _MP_SOLVER_TH_H_
#define _MP_SOLVER_TH_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Helper/error_handling.h"
#include "../Helper/Struct.h"

#include "mp_logger.h"
#include "mp_progress_disp.h"
#include "mp_consts.h"
#include "mp_thread_wrapper.h"


struct task_param_s {
	int task_type; // 0: GA, 1: kill
	uint32_t task_id;
	double* lower;
	double* upper;
	uint32_t* zone_mask;
	uint32_t* zone_id;
	//double* paramset;
	config_ga_t config_ga;
};

typedef struct task_param_s task_param_t;


struct task_queue_s {
	uint32_t queue_size;
	uint32_t current_task_id;
	thread_t* thread_id;
	task_result_queue_t* task_result_queue;
	task_param_t* task_list;
	uint32_t first_task_id;
	uint32_t next_task_id;
	thread_mutex_t* lock;
};

typedef struct task_queue_s task_queue_t;

struct fx_task_param_s {
	int task_type; // 1: FX, 255: kill
	uint32_t task_id;
	uint32_t individual_min;
	uint32_t individual_max;
	gene_pool_t* gene_pool;
	task_param_t* task_param;
};

typedef struct fx_task_param_s fx_task_param_t;

struct fx_task_queue_s {
	uint32_t first_task_id;
	uint32_t next_task_id;
	uint32_t task_size_fx;
	uint32_t queue_size;
	thread_t* thread_id;
	fx_task_param_t* fx_task_list;
	thread_mutex_t* lock;
};

typedef struct fx_task_queue_s fx_task_queue_t;


struct thread_param_s {
	int status;
	task_queue_t* task_queue;
	fx_task_queue_t* fx_task_queue;
	runtime_param_t runtime_param;
};

typedef struct thread_param_s thread_param_t;


void init_task_queue(task_queue_t* task_queue, uint32_t queue_size, task_result_queue_t* task_result_queue, uint32_t thread_count);
void free_task_queue(task_queue_t* task_queue);
void init_task(runtime_param_t runtime_param, config_ga_t config_ga, task_param_t* task);
void free_task(task_param_t* task);
void add_task(task_queue_t* task_queue, task_param_t* task);
void get_task(task_queue_t* task_queue, task_param_t* task);
void stop_task_solver_threads(task_queue_t* task_queue, uint32_t thread_count);

void init_fx_task_queue(fx_task_queue_t* fx_task_queue, uint32_t queue_size, uint32_t thread_count, uint32_t fx_task_count);
void free_fx_task_queue(fx_task_queue_t* fx_task_queue);
void add_fx_task(fx_task_queue_t* fx_task_queue, fx_task_param_t fx_task);
void get_fx_task(fx_task_queue_t* fx_task_queue, fx_task_param_t* fx_task);
void stop_fx_task_threads(fx_task_queue_t* task_queue, uint32_t thread_count);


#endif _MP_SOLVER_TH_H_



