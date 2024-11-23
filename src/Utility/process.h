
#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/multiprocessing.h"
#include "../Helper/rng.h"
#include "flatten.h"
#include "../Function/Function.h"
#include "selection.h"
#include "crossover.h"

#include "pop.h"
#include "mutation.h"

#ifndef PROCESS_H
#define PROCESS_H

// gen purpose
void process_pop(gene_pool_t* gene_pool, task_param_t* task);

#endif