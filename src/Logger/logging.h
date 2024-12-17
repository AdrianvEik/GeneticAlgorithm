
#ifndef LOGGING_H
#define LOGGING_H 

#include "../helper/struct.h"
#include "../Helper/multiprocessing.h"
#include "../Optimisation/Optimizer.h"

void copy_task_result(task_result_t* task_result, task_result_t* source);
void report_task(task_queue_t* task_queue, task_param_t* task, adaptive_memory_t* adaptive_memory, thread_param_t* thread_param, gene_pool_t* gene_pool, int best_result);
void write_file_buffer(task_result_queue_t* task_result_queue, task_result_t* task_result);
    //void write_config(gene_pool_t gene_pool, thread_param_t thread_param);
void open_file(task_result_queue_t* task_result_queue);
void close_file(task_result_queue_t* task_result_queue);

#endif // LOGGING_H