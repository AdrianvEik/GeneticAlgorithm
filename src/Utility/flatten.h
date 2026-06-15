
#ifndef FLATTEN_H
#define FLATTEN_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "..\Helper\Helper.h"
#include "..\Helper\Struct.h"
#include "..\Helper\error_handling.h"



/**
 * Transform raw fitness values into the selection-ready fitness buffer.
 *
 * The function dispatches to the configured flattening method and writes the
 * transformed values to ``gene_pool->flatten_result_set``. Selection consumes
 * this flattened buffer rather than mutating the raw objective results.
 *
 * :param gene_pool: Population state with raw fitness values.
 * :param flatten_param: Flattening method and tuning coefficients.
 */
void process_flatten(gene_pool_t* gene_pool, flatten_param_t* flatten_param);

// Flattening functions
// void lin_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void exp_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void log_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void norm_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void sig_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void no_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);

#endif
