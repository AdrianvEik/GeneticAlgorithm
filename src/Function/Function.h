
#ifndef FUNCTION_H
#define FUNCTION_H

#include <math.h>
#include <stdio.h>
#include <windows.h>

#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"

#include "../Multiprocessing/mp_thread_locals.h"
#include "../Multiprocessing/mp_solver_th.h"

/**
 * Evaluate fitness for a contiguous slice of sorted individuals.
 *
 * For double-valued objectives, the function decodes binary chromosomes into
 * ``pop_param_double`` using the task bounds before calling the built-in or
 * user-supplied objective. For integer objectives, it applies the task's zone
 * mask and zone id directly to the binary chromosomes. Fitness results are
 * multiplied by the configured optimization mode and stored in
 * ``pop_result_set``.
 *
 * :param gene_pool: Population state containing chromosomes and result arrays.
 * :param task: Task bounds, zone masks, and objective configuration.
 * :param individual_min: First sorted-individual position to evaluate.
 * :param individual_max: Last sorted-individual position to evaluate,
 *     inclusive.
 */
void process_fx_set(gene_pool_t* gene_pool, task_param_t* task, uint32_t individual_min, uint32_t individual_max);

/**
 * Evaluate all non-elite individuals for one population iteration.
 *
 * When the fitness task queue has a zero task size, evaluation happens
 * synchronously on the solver thread. Otherwise, the population is split into
 * queued fitness slices and the solver waits until the ``fx_ready`` flags show
 * all slices have completed.
 *
 * :param gene_pool: Population state to evaluate.
 * :param task: Current solver task and objective configuration.
 * :param fx_task_queue: Optional worker queue for parallel fitness slices.
 */
void process_fx(gene_pool_t* gene_pool, task_param_t* task, fx_task_queue_t* fx_task_queue);
#endif
