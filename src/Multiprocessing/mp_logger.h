
#ifndef MP_LOGGER_H
#define MP_LOGGER_H

#include <stdio.h> 

#include "mp_progress_disp.h"
#include "../Helper/Struct.h"
#include <pthread.h>

//// Forward declare pthread_t and pthread_mutex_t, since they are pointers internally
//typedef struct __pthread_mutex_t pthread_mutex_t;
//typedef unsigned long pthread_t;
//
//// Forward declarations of structs from other headers
//typedef struct runtime_param_t;  // From "../Helper/Struct.h"
//typedef struct console_queue_t; // From "mp_progress_disp.h"

struct task_result_s {
    int task_type; // 0: log, 1: best result, 255: kill
    char* csv_buffer;
    int csv_position;
    unsigned char* bin_buffer;
    int bin_position;
    double result;
    int bin_single_entry_length; // DEBUG
    int csv_single_entry_length;
};

typedef struct task_result_s task_result_t;

struct task_result_queue_s {
    task_result_t* result_list;
    pthread_t thread_id;
    FILE* fileptr;
    FILE* fileptrcsv;
    runtime_param_t runtime_param;
    console_queue_t* console_queue; // TODO: check: shared in thread_param and here?
    int first_task_id;
    int next_task_id;
    int bin_single_entry_length;
    int csv_single_entry_length;
    pthread_mutex_t* lock;
};

typedef struct task_result_queue_s task_result_queue_t;

void init_task_result_queue(task_result_queue_t* task_result_queue, runtime_param_t runtime_param, console_queue_t* console_queue);
    void free_task_result_queue(task_result_queue_t* task_result_queue);

void init_task_result(task_result_queue_t* task_result_queue, task_result_t* task_result, int entry_count);
void add_result(task_result_queue_t* task_result_queue, task_result_t* result);
void get_result(task_result_queue_t* task_result_queue, task_result_t* result);

void stop_result_logger(task_result_queue_t* task_result_queue, int thread_count, double* best_res);


#endif // MP_LOGGER_H