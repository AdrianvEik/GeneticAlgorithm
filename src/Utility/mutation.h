#include "../Helper/Helper.h"
#include "../Helper/rng.h"
#include "../Helper/Struct.h"

#ifndef MUTATION_H
#define MUTATION_H

void process_mutation(gene_pool_t* gene_pool, mutation_param_t* mutation_param);
void mutateAVXFast(gene_pool_t* gene_pool, mutation_param_t* mutation_param);
#endif