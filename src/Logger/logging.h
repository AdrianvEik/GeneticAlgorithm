
#ifndef LOGGING_H
#define LOGGING_H 

#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "../Helper/Struct.h"
#include "../Helper/Helper.h"
#include "../Helper/error_handling.h"

#include "../Optimisation/Optimizer.h"

#include "../Multiprocessing/mp_logger.h"
#include "../Multiprocessing/mp_solver_th.h"
#include "../Multiprocessing/mp_consts.h"
#include "../Multiprocessing/mp_thread_locals.h"


/**
 * Deep-copy a buffered task result into an already allocated destination.
 *
 * Used by the logging thread to keep the best result after the source task
 * buffer is freed.
 *
 * :param task_result: Destination result buffer.
 * :param source: Source result buffer to copy.
 */
void copy_task_result(task_result_t* task_result, task_result_t* source);

/**
 * Serialize the best or top-N individuals from a solver task.
 *
 * The function packs binary and CSV buffers from the current gene pool and
 * enqueues the result for the logging thread.
 *
 * :param task_queue: Solver queue whose result queue receives the report.
 * :param task: Current task metadata and bounds.
 * :param adaptive_memory: Adaptive optimizer state to include when configured.
 * :param thread_param: Solver-thread context and runtime settings.
 * :param gene_pool: Population state containing sorted results.
 * :param best_result: Non-zero when reporting the final best individual.
 */
void report_task(task_queue_t* task_queue, task_param_t* task, adaptive_memory_t* adaptive_memory, thread_param_t* thread_param, gene_pool_t* gene_pool, int best_result);

/**
 * Write one serialized result buffer to the configured CSV and/or binary files.
 *
 * :param task_result_queue: Queue containing file handles and output settings.
 * :param task_result: Serialized result to flush.
 */
void write_file_buffer(task_result_queue_t* task_result_queue, task_result_t* task_result);
    //void write_config(gene_pool_t gene_pool, thread_param_t thread_param);

/**
 * Open CSV and/or binary output files for a result queue.
 *
 * :param task_result_queue: Result queue with runtime logging settings.
 */
void open_file(task_result_queue_t* task_result_queue);

/**
 * Close CSV and/or binary output files for a result queue.
 *
 * :param task_result_queue: Result queue with open file handles.
 */
void close_file(task_result_queue_t* task_result_queue);

#endif // LOGGING_H
