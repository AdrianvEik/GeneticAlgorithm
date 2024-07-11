#include "Struct.h"

#ifndef MULTIPROCESSING_H
#define MULTIPROCESSING_H

struct task_param_s {
	int thread_id; // Owner thread
	int task_id;
	int status; // 0 = not started, 1 = in progress, 2 = completed, 3 = failed
	double* lower;
	double* upper;
	double* paramset;
	double result;
};

typedef struct task_param_s task_param_t;

struct thread_param_s {
	int thread_id;
	int task_id;
	int status;
	struct task_param_s* task_list;
	struct config_ga_s config_ga;
	struct runtime_param_s runtime_param;
};

typedef struct thread_param_s thread_param_t;


task_param_t* make_task_list(runtime_param_t* runtime_param, config_ga_t config_ga);

void free_task_list(task_param_t* task_list, runtime_param_t runtime_param);
	
#endif // MULTIPROCESSING_H