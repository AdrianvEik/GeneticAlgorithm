
#ifndef MUTATION_H
#define MUTATION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>

#include "..\Multiprocessing\mp_thread_locals.h"

#include "..\Helper\compile_flags.h"
#include "..\Helper\AVX_helper.h"
#include "..\Helper\Helper.h"
#include "..\Helper\rng.h"
#include "..\Helper\Struct.h"

void process_mutation(gene_pool_t* gene_pool, mutation_param_t* mutation_param);
void mutateAVXFast(gene_pool_t* gene_pool, mutation_param_t* mutation_param);
#endif