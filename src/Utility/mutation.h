
#ifndef MUTATION_H
#define MUTATION_H

#include "../Helper/Struct.h"
#include "../Helper/rng.h"

void mutate32(gene_pool_t* gene_pool, mutation_param_t* mutation_param, mt_rand_t* mt_rand);

#endif