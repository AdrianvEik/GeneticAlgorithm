#include "stdio.h"
#include "stdlib.h"
#include "math.h"



#include "flatten.h"

// Flattening functions
static void lin_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {

	double sum = 0;

	for (int i = 0; i < gene_pool->individuals; i++) {
		// hier min max functie
		sum += gene_pool->pop_result_set[i];
	}

	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->flatten_result_set[i] = (gene_pool->pop_result_set[i] / sum) * flatten_param->flatten_factor + flatten_param->flatten_bias;
	}

}

static void exp_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {
	/*
    mathematical function:
        f(x) = ( exp((x / sum(x)) * a) / max(exp((x / sum(x)) ) + b   clamped to [0, 1]
	where:
        a = flatten_factor *= optim_mode (-1 or 1)
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

static void log_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {

	/*
		Mathematical Function:
			f(x) = log(max(1 + ((x - b) / (range + ε)) * a, δ)) + β
		where:
			m = flatten_param->optim_mode (1 for optimization, -1 for minimization)
			b = offset (min(x) for optimization, max(x) for minimization)
			range = max(x) - min(x)
			ε = small constant to avoid division by zero
			a = flatten_factor
			β = flatten_bias
			δ = small constant to ensure valid logarithmic inputs (e.g., δ = 1e-6)

		Notes:
			- This function ensures numerical stability by clamping the logarithmic input to δ > 0.
			- For large fitness dispersions [max(x) - min(x)], smaller values of δ retain the sharp logarithmic trend.
			- Larger values of δ can smooth the transformation, flattening extreme outputs.
			- Always ensure δ << 1 to minimize distortion of the logarithmic behavior.
	*/

    // pop result set is sorted
    double min = gene_pool->pop_result_set[gene_pool->individuals - 1];
    double max = gene_pool->pop_result_set[0];
    double range = max - min;

    // hard coded eta and delta value
    double eta = 1e-6;
    double delta = 1e-6;

    for (int i = 0; i < gene_pool->individuals; i++) {
        gene_pool->flatten_result_set[i] = log(fmax(1 + ((gene_pool->pop_result_set[i] - min) / (range + eta)) * flatten_param->flatten_factor, delta)) + flatten_param->flatten_bias;
    }
}

static void norm_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {
    // min max normalization scheme 
	double min = gene_pool->pop_result_set[gene_pool->individuals - 1];
	double max = gene_pool->pop_result_set[0];
	double range = max - min;

	for (int i = 0; i < gene_pool->individuals; i++) {
        gene_pool->flatten_result_set[i] = ((gene_pool->pop_result_set[i] - min) / (range)) * flatten_param->flatten_factor + flatten_param->flatten_bias;
	}

}

static void sig_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {
    /*
        Mathematical Function:
            f(x) = 1 / (1 + exp(-x))
        where:
            x = (a * (fitness - b)) + c
            a = flatten_factor
            b = flatten_bias
            c = 0
        Notes:
            - This function is a sigmoid transformation of the fitness values.
            - The sigmoid function is bounded between 0 and 1.
            - The sigmoid function is useful for normalizing fitness values.
    */
    for (int i = 0; i < gene_pool->individuals; i++) {
        gene_pool->flatten_result_set[i] = 1 / (1 + exp(-((flatten_param->flatten_factor * (gene_pool->pop_result_set[i] - flatten_param->flatten_bias)))));
    }
}

static void no_flattening(gene_pool_t* gene_pool, flatten_param_t* flatten_param) {
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
