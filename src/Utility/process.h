
#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>


#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"
#include "../Helper/sort.h"

#include "../Multiprocessing/mp_solver_th.h"
#include "../Multiprocessing/mp_thread_locals.h"


#include "../Function/Function.h"

#include "flatten.h"
#include "selection.h"
#include "crossover.h"
#include "pop.h"
#include "mutation.h"

// gen purpose
/**
 * Execute one full genetic-operator iteration for a task.
 *
 * The iteration evaluates fitness, sorts ascending indexes, marks adjacent
 * duplicates, normalizes and flattens scores, selects parents, runs crossover
 * and mutation, then reseeds weak non-elite slots. It is the main connection
 * point between objective evaluation and the genetic operators.
 *
 * :param gene_pool: Population state to process.
 * :param task: Solver task providing bounds, zones, and configuration.
 * :param fx_task_queue: Optional queue for parallel fitness evaluation.
 */
void process_pop(gene_pool_t* gene_pool, task_param_t* task, fx_task_queue_t* fx_task_queue);

/** Record adjacent equal-score/equal-chromosome duplicates in duplicate_flags.
 * Requires ascending sorted_indexes. Preserves canonical scores and genes.
 * Returns the duplicate count used by the existing worst-slot reseed policy.
 */
uint32_t dedupe_population(gene_pool_t* gene_pool);

/**
 * Allocate thread-local pre-compute buffers used by process and selection code.
 *
 * :param gene_pool: Population dimensions used to size scratch storage.
 */
void init_pre_compute(gene_pool_t* gene_pool);

/**
 * Release thread-local pre-compute buffers allocated for the solver thread.
 */
void free_pre_compute();

#endif
