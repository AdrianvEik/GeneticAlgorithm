
#ifndef MP_LOGGER_H
#define MP_LOGGER_H

#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <pthread.h>

#include "../Helper/Struct.h"
#include "../Helper/rng.h"
#include "../Helper/error_handling.h"

#include "mp_consts.h"
#include "mp_progress_disp.h"

//// Forward declare pthread_t and pthread_mutex_t, since they are pointers internally
//typedef struct __pthread_mutex_t pthread_mutex_t;
//typedef unsigned long pthread_t;
//
//// Forward declarations of structs from other headers
//typedef struct runtime_param_t;  // From "../Helper/Struct.h"
//typedef struct console_queue_t; // From "mp_progress_disp.h"

struct task_result_s {
    char* csv_buffer;
    unsigned char* bin_buffer;
    double result;
    uint32_t iteration;
    uint32_t bin_position;
    uint32_t csv_position;
    uint32_t bin_buffer_length; // DEBUG
    uint32_t csv_buffer_length;
    int task_type; // 0: log, 1: best result, 255: kill
};

typedef struct task_result_s task_result_t;

struct progress_s {
    double elapsed_time;
    double best_result;
    double average_result;
    double result_standard_deviation;
    uint32_t tasks_completed;
    uint32_t max_tasks;
    uint32_t best_result_iteration;
    int optim_mode;
};

typedef struct progress_s progress_t;

struct task_result_queue_s {
    task_result_t* result_list;
    pthread_t thread_id;
    FILE* fileptr;
    FILE* fileptrcsv;
    runtime_param_t runtime_param;
    fx_param_t fx_param;
    console_queue_t* console_queue; // TODO: check: shared in thread_param and here?
    progress_t progress;
    uint32_t first_task_id;
    uint32_t next_task_id;
    uint32_t bin_single_entry_length;
    uint32_t csv_single_entry_length;
    pthread_mutex_t* lock;
};

typedef struct task_result_queue_s task_result_queue_t;

void init_task_result_queue(task_result_queue_t* task_result_queue, runtime_param_t runtime_param, console_queue_t* console_queue, fx_param_t fx_param);
void free_task_result_queue(task_result_queue_t* task_result_queue);

void init_task_result(task_result_queue_t* task_result_queue, task_result_t* task_result, uint32_t entry_count);
void add_result(task_result_queue_t* task_result_queue, task_result_t* result);
void free_task_result(task_result_t* result);
int get_result(task_result_queue_t* task_result_queue, task_result_t* result);

void stop_result_logger(task_result_queue_t* task_result_queue, uint32_t thread_count, double* best_res);


#endif // MP_LOGGER_H