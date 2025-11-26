

#ifndef CROSSOVER_H
#define CROSSOVER_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#include "..\Helper\AVX_helper.h"
#include "..\Helper\Helper.h"
#include "..\Helper\Struct.h"
#include "..\Helper\rng.h"

void process_crossover(gene_pool_t* gene_pool, crossover_param_t* crossover_param);

#endif