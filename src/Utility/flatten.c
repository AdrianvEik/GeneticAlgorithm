#include "stdio.h"
#include "stdlib.h"
#include "math.h"



#include "flatten.h"

// Flattening functions
void lin_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {

	double sum = 0;

	for (int i = 0; i < gene_pool->individuals; i++) {
		// hier min max functie
		sum += gene_pool->pop_result_set[i];
	}

	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->flatten_result_set[i] = (gene_pool->pop_result_set[i] / sum) * flatten_param->flatten_factor + flatten_param->flatten_bias;
	}

}

void exp_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {
	/*
    mathematical function:
        f(x) = ( exp((x / sum(x)) * a) / max(exp((x / sum(x)) ) + b   clamped to [0, 1]
	where:
        a = flatten_factor
        b = flatten_bias
        x = fitness value

	Note:
		For large fitness dispersions [fitness_max - fitness_min] the result might stick to the 
        domain edge [0, 1] for various values close to fitness_max (or fitness_min).
		To avoid this, use sigmoid flattening.
	*/

	double sum = 0;
	for (int i = 0; i < gene_pool->individuals; i++) {
		sum += gene_pool->pop_result_set[i];
	}

	double max_exp = 0.0;
	// compute normalized exp values
    for (int i = 0; i < gene_pool->individuals; i++) {
        gene_pool->flatten_result_set[i] = exp((gene_pool->pop_result_set[i] / sum) * flatten_param->flatten_factor);
		if (gene_pool->flatten_result_set[i] > max_exp) {
			max_exp = gene_pool->flatten_result_set[i];
		}
    }

    // normalize and bias
    for (int i = 0; i < gene_pool->individuals; i++) {
        gene_pool->flatten_result_set[i] = (gene_pool->flatten_result_set[i] / max_exp) + flatten_param->flatten_bias;
        // clamp to domain [0, 1] TODO: make variable?
        if (gene_pool->flatten_result_set[i] < 0) {
            gene_pool->flatten_result_set[i] = 0;
        }
        else if (gene_pool->flatten_result_set[i] > 1) {
            gene_pool->flatten_result_set[i] = 1;
        }
    }
}

void log_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {

	/*

	Compute:

	->-> math::
		f(x) = a^\log(\dfrac{x}{\mathrm{sum}(x)) + b

	For all fitness values in pop (individuals)

	:param pop: matrix of fitness values or

	*/

	double sum = 0;

	for (int i = 0; i < gene_pool->individuals; i++) {
		sum += gene_pool->pop_result_set[i];
	}
	double lgda = log(flatten_param->flatten_factor);
	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->flatten_result_set[i] = log((gene_pool->pop_result_set[i] / sum)) / lgda + flatten_param->flatten_bias;
	}
}

void norm_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {

	double sum = 0;

	for (int i = 0; i < gene_pool->individuals; i++) {
		sum += gene_pool->pop_result_set[i];
	}

	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->flatten_result_set[i] = gene_pool->pop_result_set[i] / sum;
	}

}

void sig_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {

}

void no_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {
	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->flatten_result_set[i] = gene_pool->pop_result_set[i];
	}
}


void process_flatten(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {
	/*

	:param pop: matrix of fitness values or

	*/
	// // copy the 

	// if(gene_pool->fx_param->fx_optim_mode == 0){
	//         for(int i = 0; i< gene_pool->individuals; i++){
	//                 pop_result_set[i] = -pop_result_set[i];
	//         }

	// }
	// else{
	//         printf("Error: mode is not 0 or 1\n");
	//         exit(1);
	// }


	if (flatten_param->flatten_method == flatten_method_linear) {
		lin_flattening(gene_pool, flatten_param);
	}
	else if (flatten_param->flatten_method == flatten_method_exponential) {
		exp_flattening(gene_pool, flatten_param);
	}
	else if (flatten_param->flatten_method == flatten_method_logarithmic) {
		log_flattening(gene_pool, flatten_param);
	}
	else if (flatten_param->flatten_method == flatten_method_normalized) {
		norm_flattening(gene_pool, flatten_param);
	}
	else if (flatten_param->flatten_method == flatten_method_sigmoid) {
		sig_flattening(gene_pool, flatten_param);
	}
	else if (flatten_param->flatten_method == flatten_method_none) {
		no_flattening(gene_pool, flatten_param);
	}
	else {
		printf("Error: flatten_method is not 0, 1, 2, 3, 4 or 5\n");
	}

}
