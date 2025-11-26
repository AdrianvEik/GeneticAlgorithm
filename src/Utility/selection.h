#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"

#ifndef SELECTION_H
#define SELECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <pthread.h>
#include "../Multiprocessing/mp_thread_locals.h"
#include "../Helper/error_handling.h"

// Selection functions



// gen purpose

void process_selection(gene_pool_t* gene_pool, selection_param_t* selection_param);
void init_pre_compute_selection(gene_pool_t* gene_pool);
void free_pre_compute_selection();
#endif

// Selection functions
 //void roulette_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);
 //void rank_tournament_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);
 //void rank_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);
 //void rank_space_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);
 //void boltzmann_selection(gene_pool_t *gene_pool, selection_param_t *selection_param);

