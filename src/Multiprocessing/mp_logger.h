
#ifndef MP_LOGGER_H
#define MP_LOGGER_H

#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#include "../Helper/Struct.h"
#include "../Helper/rng.h"
#include "../Helper/error_handling.h"

#include "mp_consts.h"
#include "mp_progress_disp.h"
#include "mp_thread_wrapper.h"

//// Forward declare thread_t and thread_mutex_t, since they are pointers internally
//typedef struct __thread_mutex_t thread_mutex_t;
//typedef unsigned long thread_t;
//
//// Forward declarations of structs from other headers
//typedef struct runtime_param_t;  // From "../Helper/Struct.h"
//typedef struct console_queue_t; // From "mp_progress_disp.h"

/**
 * Serialized result produced by a solver task and consumed by the logger.
 *
 * The buffers contain one or more top-N entries in binary and optional CSV
 * form. ``task_type`` distinguishes normal log records, final best records,
 * and termination records.
 */
struct task_result_s {
    /** Optional CSV row buffer. */
    char* csv_buffer;
    /** Binary row buffer. */
    unsigned char* bin_buffer;
    /** Best or reported objective value for this record. */
    double result;
    /** Generation at which the result was reported. */
    uint32_t iteration;
    /** Current write position in ``bin_buffer``. */
    uint32_t bin_position;
    /** Current write position in ``csv_buffer``. */
    uint32_t csv_position;
    /** Allocated byte length of ``bin_buffer``. */
    uint32_t bin_buffer_length;
    /** Allocated byte length of ``csv_buffer``. */
    uint32_t csv_buffer_length;
    /** Result kind, such as ``LOG_TASK``, ``BEST_RESULT_TASK``, or terminate. */
    int task_type;
};

typedef struct task_result_s task_result_t;

/**
 * Aggregated progress visible to callers and the terminal display.
 *
 * The logging thread updates this struct as solver tasks complete and copies
 * it to the caller-provided ``progress_t`` at the end of
 * :c:func:`Genetic_Algorithm`.
 */
struct progress_s {
    /** Elapsed wall-clock time tracked by the logger. */
    double elapsed_time;
    /** Best result observed across completed solver tasks. */
    double best_result;
    /** Sum of completed task results, used to compute the average. */
    double average_result;
    /** Accumulated variance term for completed task results. */
    double result_standard_deviation;
    /** Number of solver tasks that reported a final best result. */
    uint32_t tasks_completed;
    /** Expected number of solver tasks. */
    uint32_t max_tasks;
    /** Iteration number at which the best result was reported. */
    uint32_t best_result_iteration;
    /** Optimization direction associated with the progress record. */
    int optim_mode;
};

typedef struct progress_s progress_t;

/**
 * Ring buffer and file context for serialized task results.
 *
 * Solver threads enqueue :c:type:`task_result_t` values, the logging thread
 * drains them, writes configured files, updates progress, and stores the best
 * final result.
 */
struct task_result_queue_s {
    /** Ring-buffer storage for serialized task results. */
    task_result_t* result_list;
    /** Logger thread handle. */
    thread_t thread_id;
    /** Binary output file handle. */
    FILE* fileptr;
    /** CSV output file handle. */
    FILE* fileptrcsv;
    /** Runtime settings used for queue and output sizing. */
    runtime_param_t runtime_param;
    /** Fitness settings used to choose serialized row layout. */
    fx_param_t fx_param;
    /** Console queue drained by the logger for progress messages. */
    console_queue_t* console_queue;
    /** Aggregate progress updated by the logger. */
    progress_t progress;
    /** Read cursor in ``result_list``. */
    uint32_t first_task_id;
    /** Write cursor in ``result_list``. */
    uint32_t next_task_id;
    /** Binary byte count for one exported individual. */
    uint32_t bin_single_entry_length;
    /** CSV byte estimate for one exported individual. */
    uint32_t csv_single_entry_length;
    /** Mutex protecting queue cursors and storage. */
    thread_mutex_t* lock;
};

typedef struct task_result_queue_s task_result_queue_t;

/**
 * Initialize the task-result queue used by solver and logging threads.
 *
 * :param task_result_queue: Queue struct to initialize.
 * :param runtime_param: Runtime settings used for queue and buffer sizing.
 * :param console_queue: Console-message queue shared with the logger.
 * :param fx_param: Fitness settings used to size serialized records.
 */
void init_task_result_queue(task_result_queue_t* task_result_queue, runtime_param_t runtime_param, console_queue_t* console_queue, fx_param_t fx_param);

/**
 * Release result-queue storage and synchronization primitives.
 *
 * :param task_result_queue: Initialized result queue to release.
 */
void free_task_result_queue(task_result_queue_t* task_result_queue);

/**
 * Allocate CSV and binary buffers for a result record.
 *
 * :param task_result_queue: Queue whose sizing metadata controls allocation.
 * :param task_result: Result record to initialize.
 * :param entry_count: Number of individual records stored in the buffers.
 */
void init_task_result(task_result_queue_t* task_result_queue, task_result_t* task_result, uint32_t entry_count);

/**
 * Push a serialized result onto the logging queue.
 *
 * :param task_result_queue: Destination result queue.
 * :param result: Result record to enqueue by value.
 */
void add_result(task_result_queue_t* task_result_queue, task_result_t* result);

/**
 * Free buffers allocated for a result record.
 *
 * :param result: Result record initialized by :c:func:`init_task_result`.
 */
void free_task_result(task_result_t* result);

/**
 * Try to pop one serialized result from the logging queue.
 *
 * :param task_result_queue: Queue to read from.
 * :param result: Destination result record filled on success.
 * :returns: ``1`` when a result was read, otherwise ``0``.
 */
int get_result(task_result_queue_t* task_result_queue, task_result_t* result);

/**
 * Signal the result logger to stop and wait for it to finish.
 *
 * :param task_result_queue: Logging queue and logger-thread handle.
 * :param thread_count: Number of solver threads that can send termination
 *     records.
 * :param best_res: Destination for the best result tracked by the logger.
 */
void stop_result_logger(task_result_queue_t* task_result_queue, uint32_t thread_count, double* best_res);


#endif // MP_LOGGER_H
