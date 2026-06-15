#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"

#ifndef SELECTION_H
#define SELECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include "../Multiprocessing/mp_thread_locals.h"
#include "../Helper/error_handling.h"

// Selection functions



// gen purpose

/**
 * Select parent indexes for the next generation.
 *
 * The dispatcher reads ``flatten_result_set`` and writes chosen source indexes
 * into ``selected_indexes``. Available strategies include roulette,
 * tournament, rank, rank-space, and Boltzmann selection.
 *
 * :param gene_pool: Population state containing sorted and flattened results.
 * :param selection_param: Selection method and probability-shaping controls.
 */
void process_selection(gene_pool_t* gene_pool, selection_param_t* selection_param);

/**
 * Allocate thread-local scratch arrays used by selection routines.
 *
 * :param gene_pool: Population dimensions used to size selection workspaces.
 */
void init_pre_compute_selection(gene_pool_t* gene_pool);

/**
 * Release thread-local selection scratch arrays.
 */
void free_pre_compute_selection();
#endif

// Selection functions
 //void roulette_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);
 //void rank_tournament_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);
 //void rank_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);
 //void rank_space_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);
 //void boltzmann_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);

