
#include <math.h>
#include <stdio.h>

#include "Function.h"

#include "../Helper/Helper.h"
#include "../Helper/Struct.h"


double Styblinski_Tang_fx(double* parameter_set, int genes) {
	double result = 0;
	for (int i = 0; i < genes; i++) {
		result += (pow(parameter_set[i], 4)) - (16 * pow(parameter_set[i], 2)) + (5 * parameter_set[i]);
	}
	return result / 2;
}

// def wheelers_ridge(x: Union[np.ndarray, list], a: float = 1.5) -> float:
//     """
//     Compute the Wheelersridge function for given x1 and x2

//     :param x: list with x1 (otype: float) and x2 (otype: float)
//     :param a: additional parameter typically a=1.5

//     :return: Value f(x1, x2, a), real float
//     """
//     x1, x2 = x
//     return -np.exp(-(x1 * x2 - a) ** 2 - (x2 - a) ** 2)

double wheelers_ridge_fx(double* parameter_set, int genes) {
	double a = 1.5;

	// check if genes = 2
	if (genes != 2) {
		printf("Error: Wheelers Ridge function requires 2 genes\n");
		return 0;
	}

	double x1 = parameter_set[0];
	double x2 = parameter_set[1];
	return -1 * exp(-1 * pow(x1 * x2 - a, 2) - pow(x2 - a, 2));
}

void process_fx(gene_pool_t* gene_pool, fx_param_t* fx_param, double* lower, double* upper) {
	/*

	:param pop: matrix of individuals as double (individuals x genes)
	:param individuals: number of individuals
	:param genes: number of genes
	:param fx: fitness function (double array x0 x1 ... xn)
	:param result: matrix of fitness values (individuals x 1)

	*/

	// convert the gene pool bin to double
	ndbit2int32(gene_pool->pop_param_bin, gene_pool->genes, gene_pool->individuals, lower, upper, gene_pool->pop_param_double);

	if (fx_param->fx_method == fx_method_Styblinski_Tang) {
		fx_param->fx_optim_mode = -1;
		for (int i = 0; i < gene_pool->individuals; i++) {
			gene_pool->pop_result_set[i] = fx_param->fx_optim_mode * Styblinski_Tang_fx(gene_pool->pop_param_double[i], gene_pool->genes);
		}
	}
	else if (fx_param->fx_method == fx_method_Wheelers_Ridge) {
		fx_param->fx_optim_mode = -1;
		for (int i = 0; i < gene_pool->individuals; i++) {
			gene_pool->pop_result_set[i] = fx_param->fx_optim_mode * wheelers_ridge_fx(gene_pool->pop_param_double[i], gene_pool->genes);
		}
	}
    else if (fx_param->fx_method == fx_method_pointer) {
        if (fx_param->fx_function != NULL) {
            for (int i = 0; i < gene_pool->individuals; i++) {
                gene_pool->pop_result_set[i] = fx_param->fx_optim_mode * fx_param->fx_function(gene_pool->pop_param_double[i], gene_pool->genes);
            }
        }
        else {
            printf("Error: Function pointer is NULL\n");
        }
    }
	else {
		printf("Error: Unknown fitness function\n");
	}
}
