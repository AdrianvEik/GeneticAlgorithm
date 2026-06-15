
#ifndef _POP_H
#define _POP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <float.h>

#include "../Helper/error_handling.h"
#include "../Helper/compile_flags.h"
#include "../Helper/AVX_helper.h"
#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"


//#define pop_cauchy 2

/**
 * Allocate and wire the contiguous memory backing a gene pool.
 *
 * The allocator creates aligned storage for binary chromosomes, crossover
 * buffers, decoded doubles, result arrays, sorted-index workspaces, and
 * fitness-completion flags. The pool dimensions are copied from
 * ``runtime_param``.
 *
 * :param gene_pool: Gene pool struct to initialize.
 * :param runtime_param: Runtime dimensions and bit-width settings.
 */
void init_gene_pool(gene_pool_t* gene_pool, runtime_param_t* runtime_param);

/**
 * Release memory allocated by :c:func:`init_gene_pool`.
 *
 * :param gene_pool: Initialized gene pool to release.
 */
void free_gene_pool(gene_pool_t* gene_pool);

/**
 * Seed a gene pool population and reset initial result bookkeeping.
 *
 * The selected population sampler fills ``pop_param_bin`` and initializes
 * every fitness result to the worst sentinel for the configured optimization
 * mode.
 *
 * :param gene_pool: Allocated population storage to fill.
 * :param pop_param: Population sampling configuration.
 * :param fx_param: Fitness settings used to choose minimization/maximization
 *     sentinels.
 */
void fill_pop(gene_pool_t* gene_pool, population_param_t pop_param, fx_param_t fx_param);

/**
 * Fill one individual with uniformly distributed random chromosome bits.
 *
 * The implementation uses the widest available project RNG vector path
 * selected by compile flags.
 *
 * :param gene_pool: Gene pool containing the binary population buffer.
 * :param individual: Index of the individual to overwrite.
 */
void fill_individual_uniform(gene_pool_t* gene_pool, uint32_t individual);

#endif // _POP_H
