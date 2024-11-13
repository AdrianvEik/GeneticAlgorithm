
#ifndef CROSSOVER_H
#define CROSSOVER_H

#include "../Helper/Struct.h"
#include "../Helper/rng.h"

static const int crossover_method_single_point32 = 0;
static const int crossover_method_two_point32 = 1;
static const int crossover_method_uniform32 = 2;
static const int crossover_method_complete32 = 3;

void process_crossover(gene_pool_t* gene_pool, crossover_param_t* crossover_param);

#endif