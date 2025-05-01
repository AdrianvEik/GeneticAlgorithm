
#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"

#include "../Multiprocessing/mp_solver_th.h"

#include "../Function/Function.h"

#include "flatten.h"
#include "selection.h"
#include "crossover.h"
#include "pop.h"
#include "mutation.h"

// gen purpose
void process_pop(gene_pool_t* gene_pool, task_param_t* task);

void init_pre_compute(gene_pool_t* gene_pool);
void free_pre_compute();

#endif