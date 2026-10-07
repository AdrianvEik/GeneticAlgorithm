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
 * The dispatcher writes physical source indexes into ``selected_indexes``.
 * Roulette reads finite nonnegative flatten_result_set; tournament and
 * Boltzmann read normalized_result_set. Rank methods map ascending ranks
 * through sorted_indexes; rank-space mixes rank and encoded-coordinate diversity.
 * Existing IDs 0..4 are retained; pairwise logistic=5, strict Boltzmann=6,
 * relaxed Boltzmann=7. ID 4 uses the paper's 50/50 strict/relaxed mixture.
 * Preconditions: population/elitism valid, tournament size >=1, p and lambda
 * in [0,1], positive finite temperature, finite class threshold in [0,1], and
 * rank distribution 0 (geometric) or 1 (exponential). Parameter validation
 * is deferred to the project-wide sweep; no settings are silently adjusted.
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

