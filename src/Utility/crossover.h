

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

/**
 * Recombine selected parent chromosomes into the crossover buffer.
 *
 * The dispatcher walks pairs from ``selected_indexes`` and applies the
 * configured crossover strategy. Elite individuals are preserved by the
 * surrounding population-processing pipeline.
 *
 * :param gene_pool: Population state containing parent indexes and binary
 *     chromosome buffers.
 * :param crossover_param: Crossover method and probability controls.
 */
void process_crossover(gene_pool_t* gene_pool, crossover_param_t* crossover_param);

#endif
