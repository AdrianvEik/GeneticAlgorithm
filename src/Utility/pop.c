#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#define PI   3.14159265358979323846264338327950288419716939937510f

#include "pop.h"
#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"

void bitpop32(int genes, int* result) {

	/*
	Fill a vector with uniformly distributed random bits.

	:param genes: The number of genes in an individual.
	:type genes: int


	:param result: The matrix to be filled with random bits.
				   shape = (genes )
	:type result: int*

	*/

	for (int j = 0; j < genes; j++) {
		result[j] = gen_mt_rand();
	}

}

inline uint32_t double2bin(double val, double lower, double upper) {
    /*
    Convert an integer to a double. andersom!

    :param val: The integer to be converted to a double.
    :type val: int

    :return: The double.
    :rtype: int
    */	
	return ((val - lower) / (upper - lower)) * UINT32_MAX;
}

void normal_bit_pop_boxmuller(int** result, int individuals, int genes) {
	/*
	Fill a matrix with bits according to a normal distribution.
	using the following probability density function:

	.. math::
		f(x) = \\frac{1}{\\sigma \\sqrt{2 \\pi}} e^{-\\frac{1}{2} (\\frac{x - \\mu}{\\sigma})^2}

	Calculate them using a Box-Muller transform, where two random numbers are generated
	according to a uniform distribution and then transformed to a normal distribution with
	the following formula:

	.. math::
		z_0 = \sqrt{-2 \ln U_1 } \cos{(2 \pi U_2)} \\
		z_1 = \sqrt{-2 \ln U_1 } \sin{(2 \pi U_2)}

	Where :math:`U_1` and :math:`U_2` are random numbers picked from a uniform distribution.

	:param genes: The number of genes in the bitstring.
	:type genes: int

	:param individuals: The number of individuals in the bitstring.
	:type individuals: int

	:param result: The matrix to be filled with bits according to a normal distribution.
				   shape = (individuals, genes * bitsize)
	:type result: int**
	*/
	double U1, U2;
	double z1, z2;

	// Error currently seems to be located in the latter half of the genes
	for (int i = 0; i < individuals; i++) {
		for (int j = 0; j < genes; j += 2) { 
            U1 = ((double) gen_mt_rand() / UINT32_MAX);
            U2 = ((double) gen_mt_rand() / UINT32_MAX);

			// ln 0 = inf
            if (U1 == 0) {
                U1 = 1;
            }

			z1 = sqrt(-2 * log(U1)) * cos(2 * PI * U2);
			z2 = sqrt(-2 * log(U1)) * sin(2 * PI * U2); 

            // Box muller generates normalised values between -6.7 and 6.7 (using int32 resolution)
			result[i][j] = double2bin((z1), -6.7, 6.7);
            if (j <= genes) { // Check if the next gene is within the bounds of the genes
				result[i][j+1] = double2bin((z2), -6.7, 6.7);
			}
		}
        
	}
}

void cauchy_bit_pop(int** result, int individuals, int genes, population_param_t pop_param) {
	/*

	Produce a normal distributed set of values using the Cauchy distribution:

	.. math::
		f(x) = \frac{1}{\pi \gamma [1 + (\frac{x - x_0}{\gamma})^2]}

	Where x is linearly spaced between (-factor and factor) + bias.

	:param bitsize: The size of the bitstring.
	:type bitsize: int

	:param genes: The number of genes in the bitstring.
	:type genes: int

	:param individuals: The number of individuals in the bitstring.
	:type individuals: int

	:param result: The matrix to be filled with bits according to a normal distribution.
				   shape = (individuals, genes * bitsize)
	:type result: int**

	*/
	// Determine the steps between the values in the normal distribution
		// make scale and loc in for loop
	double scale;
	double loc;

	double cauchydouble;
	double scaledcauchy;

	for (int i = 0; i < individuals; i++) {
		for (int j = 0; j < genes; j++) {
			scale = 1 / pop_param.sigma * (pop_param.upper[i] - pop_param.lower[i]) / 2;
			loc = (pop_param.upper[i] + pop_param.lower[i]) / 2;
			
			cauchydouble = cauchy((gen_mt_rand() << 32) | gen_mt_rand(), 0, 1);
            scaledcauchy = (cauchydouble * scale) + loc;
			
			result[i][j] = double2bin(scaledcauchy, pop_param.lower[i], pop_param.upper[i]);
		}
	}
}

void init_gene_pool(gene_pool_t* gene_pool) {
	//gene_pool_t {
	// int** pop_param_bin;
	// double** pop_param_double;
	// double* pop_result_set;
	// int* selected_indexes;
	// int genes;
	// int individuals;
	// int elitism;

	if ((gene_pool->flatten_result_set = malloc(gene_pool->individuals * sizeof(double))) == NULL ||
		(gene_pool->pop_param_bin = (int**)malloc(gene_pool->individuals * sizeof(int*))) == NULL ||
		(gene_pool->pop_param_bin_cross_buffer = (int**)malloc(gene_pool->individuals * sizeof(int*))) == NULL ||
		(gene_pool->pop_param_double = malloc(gene_pool->individuals * sizeof(double*))) == NULL ||
		(gene_pool->pop_result_set = malloc(gene_pool->individuals * sizeof(double))) == NULL ||
		(gene_pool->selected_indexes = malloc(gene_pool->individuals * sizeof(int))) == NULL ||
		(gene_pool->sorted_indexes = malloc(gene_pool->individuals * sizeof(int))) == NULL) {
		fprintf(stderr, "Memory allocation failed: init_gene_pool\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < gene_pool->individuals; i++) {
		if ((gene_pool->pop_param_bin[i] = (int*)malloc(gene_pool->genes * sizeof(int))) == NULL ||
			(gene_pool->pop_param_bin_cross_buffer[i] = (int*)malloc(gene_pool->genes * sizeof(int))) == NULL ||
			(gene_pool->pop_param_double[i] = (double*)malloc(gene_pool->genes * sizeof(double))) == NULL) {
			fprintf(stderr, "Memory allocation failed: init_gene_pool individual %d\n", i);
			exit(EXIT_FAILURE);
		}
	}
}
void free_gene_pool(gene_pool_t* gene_pool) {
	for (int i = 0; i < gene_pool->individuals; i++) {
		free(gene_pool->pop_param_bin[i]);
		free(gene_pool->pop_param_bin_cross_buffer[i]);
		free(gene_pool->pop_param_double[i]);
	}
	free(gene_pool->flatten_result_set);
	free(gene_pool->pop_param_bin);
	free(gene_pool->pop_param_bin_cross_buffer);
	free(gene_pool->pop_param_double);
	free(gene_pool->pop_result_set);
	free(gene_pool->selected_indexes);
	free(gene_pool->sorted_indexes);
}

void fill_individual(gene_pool_t* gene_pool, int individual) {
	bitpop32(gene_pool->genes, gene_pool->pop_param_bin[individual]);
}

void fill_pop(gene_pool_t* gene_pool, population_param_t pop_param) {
	if (pop_param.sampling_type == pop_uniform)
		for (int i = 0; i < gene_pool->individuals; i++) {
			fill_individual(gene_pool, i);
		}
	else if (pop_param.sampling_type == pop_normal) {
		normal_bit_pop_boxmuller(gene_pool->pop_param_bin, gene_pool->individuals, gene_pool->genes);
	}
	else if (pop_param.sampling_type == pop_cauchy) {
		cauchy_bit_pop(gene_pool->pop_param_bin, gene_pool->individuals, gene_pool->genes, pop_param);
	}
}