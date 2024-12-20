#include <pthread.h>
#include <stdio.h>

#include "../Helper/Struct.h"
#include "../Helper/rng.h"

#ifndef MULTIPROCESSING_H
#define MULTIPROCESSING_H

static const int TERMINATE_THREAD = 255;

static const int LOG_TASK = 0;
static const int BEST_RESULT_TASK = 1;

static const int GA_TASK = 0;

struct task_param_s {
	int task_type; // 0: GA, 1: kill
    int task_id;
	double* lower;
	double* upper;
	//double* paramset;
    struct config_ga_s config_ga;
};

typedef struct task_param_s task_param_t;

struct task_result_s {
    int task_type; // 0: log, 1: best result, 255: kill
    char* csv_buffer;
    int csv_position;
    unsigned char* bin_buffer;
    int bin_position;
	double result;
	int bin_single_entry_length; // DEBUG
	int csv_single_entry_length;
 //   
 //   int task_id;
 //   int individual_id;
 //   int position;
	//int converged;
	//
	//double* lower;
 //   double* upper;
 //   
	//int* config_int;
 //   double* config_double;
};

typedef struct task_result_s task_result_t;

// TODO: is this the best way to handle progress?
struct progress_s {
    double best_result;
    int tasks_completed;
	int optim_mode;
    int kill_progress_display; // 0: continue, 1: kill
};

typedef struct progress_s progress_t;

struct task_result_queue_s {
    task_result_t* result_list;
	pthread_t thread_id;
    pthread_t thread_id_progress_display;
	FILE* fileptr;
	FILE* fileptrcsv;
	runtime_param_t runtime_param;
    progress_t progress;
	int first_task_id;
    int next_task_id;
    int bin_single_entry_length;
    int csv_single_entry_length;
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

void init_task_result(task_result_queue_t* task_result_queue, task_result_t* task_result, int entry_count);
void free_task_result(task_result_t* task_result);


void init_task_result_queue(task_result_queue_t* task_result_queue,runtime_param_t runtime_param);
void add_result(task_result_queue_t* task_result_queue, task_result_t* result);
void get_result(task_result_queue_t* task_result_queue, task_result_t* result);
void stop_result_logger(task_result_queue_t* task_result_queue, int thread_count);

void init_task_queue(task_queue_t* task_queue,int queue_size, task_result_queue_t* task_result_queue, int thread_count);
void free_task_queue(task_queue_t* task_queue);
void add_task(task_queue_t* task_queue, task_param_t* task);
void get_task(task_queue_t* task_queue, task_param_t* task);
void stop_threads(task_queue_t* task_queue, int thread_count);

#endif // MULTIPROCESSING_H