#include "../Helper/Helper.h"
#include "../Helper/Struct.h"

#ifndef FLATTEN_H
#define FLATTEN_H

static const int flatten_method_linear = 0;
static const int flatten_method_exponential = 1;
static const int flatten_method_logarithmic = 2;
static const int flatten_method_normalized = 3;
static const int flatten_method_sigmoid = 4;
static const int flatten_method_none = 5;

void process_flatten(gene_pool_t* gene_pool, flatten_param_t* flatten_param);

// Flattening functions
// void lin_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void exp_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void log_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void norm_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void sig_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);
// void no_flattening(gene_pool_t gene_pool, flatten_param_t flatten_param, int* result);

#endif
