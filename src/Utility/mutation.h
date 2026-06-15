
#ifndef MUTATION_H
#define MUTATION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdalign.h>
#include <immintrin.h>

#include "..\Multiprocessing\mp_thread_locals.h"

#include "..\Helper\compile_flags.h"
#include "..\Helper\AVX_helper.h"
#include "..\Helper\Helper.h"
#include "..\Helper\rng.h"
#include "..\Helper\Struct.h"

/**
 * Mutate non-elite chromosomes in-place.
 *
 * For each sorted non-elite individual, this function derives a per-block bit
 * flipping probability from ``mutation_rate`` and applies random XOR masks to
 * the binary chromosome. Mutation rates are typically updated by
 * :c:func:`adapt_param` before this function runs.
 *
 * :param gene_pool: Population state containing sorted binary chromosomes.
 * :param mutation_param: Mutation probabilities and per-individual mutation
 *     rates.
 */
void process_mutation(gene_pool_t* gene_pool, mutation_param_t* mutation_param);
#endif
