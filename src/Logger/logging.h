
#ifndef LOGGING_H
#define LOGGING_H 

#include "../helper/struct.h"

void write_param(task_result_queue_t task_result_queue, task_result_t task_result);
//void write_config(gene_pool_t gene_pool, thread_param_t thread_param);
void open_file(task_result_queue_t* task_result_queue);
void close_file(task_result_queue_t* task_result_queue);

#endif // LOGGING_H