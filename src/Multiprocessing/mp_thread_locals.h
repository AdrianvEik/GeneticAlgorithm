
#ifndef _MP_THREAD_LOCALS_H
#define _MP_THREAD_LOCALS_H

#include "../Helper/Struct.h"

void init_pre_compute(gene_pool_t* gene_pool);
void free_pre_compute();

// RNG
// has a thread local seed struct for each thread

// Selection parameters
__declspec(thread) double* prob_distr;
__declspec(thread) double* boltzmann_distr;
__declspec(thread) double current_prob_param;
__declspec(thread) double current_temp_param;

// In case of using the rank_space selection method
__declspec(thread) double* distances;
__declspec(thread) double* central_point;

// Mutation parameters
__declspec (thread) int* muation_boost_distr;
__declspec (thread) double current_alpha;
__declspec (thread) double current_beta;

#endif // !_MP_THREAD_LOCALS_H