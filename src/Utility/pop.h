
#ifndef _POP_H
#define _POP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>

#include "../Helper/error_handling.h"
#include "../Helper/compile_flags.h"
#include "../Helper/AVX_helper.h"
#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"

#define pop_uniform 0
#define pop_normal 1
//#define pop_cauchy 2

void init_gene_pool(gene_pool_t* gene_pool, runtime_param_t* runtime_param);
void free_gene_pool(gene_pool_t* gene_pool);

void fill_pop(gene_pool_t* gene_pool, population_param_t pop_param);
void fill_individual_uniform(gene_pool_t* gene_pool, int individual);

#endif // _POP_H