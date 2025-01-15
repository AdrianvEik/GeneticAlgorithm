
#ifndef MP_THREAD_LOCALS_H
#define MP_THREAD_LOCALS_H

#include "../Helper/Struct.h"

#define thread_local __declspec( thread )

void init_pre_compute(gene_pool_t* gene_pool);
void free_pre_compute();

// RNG
// has a thread local seed struct for each thread

// Selection parameters
extern thread_local double* prob_distr;
extern thread_local double* boltzmann_distr;
extern thread_local double current_prob_param;
extern thread_local double current_temp_param;

// In case of using the rank_space selection method
extern thread_local double* distances;
extern thread_local double* central_point;

// Mutation parameters
extern thread_local int* muation_boost_distr;
extern thread_local double current_alpha;
extern thread_local double current_beta;

#endif // !MP_THREAD_LOCALS_H