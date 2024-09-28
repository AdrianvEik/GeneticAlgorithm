#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#define PI   3.14159265358979323846264338327950288419716939937510f

#include "pop.h"
#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/rng.h"

void bitpop32(int genes, int* result, mt_rand_t* mt_rand) {

	/*
	Fill a vector with uniformly distributed random bits.

	:param genes: The number of genes in an individual.
	:type genes: int


	:param result: The matrix to be filled with random bits.
				   shape = (genes )
	:type result: int*

	*/

	for (int j = 0; j < genes; j++) {
		result[j] = gen_mt_rand(mt_rand);
	}

}

inline uint32_t float2bin(double val, double lower, double upper) {
    /*
    Convert an integer to a bitarray.

    :param val: The integer to be converted to a bitarray.
    :type val: int

    :return: The bitarray.
    :rtype: int
    */	
	return ((val - lower) / (upper - lower)) * pow(2, 8 * sizeof(uint32_t));
}

void normal_bit_pop_boxmuller(int** result, int individuals, int genes, population_param_t pop_param, mt_rand_t* mt_rand) {
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

	// make scale and loc in for loop
	double scale;
	double loc;
    uint8_t extra = genes % 2;
	double z1, z2;
	float U1, U2;
	uint32_t res1, res2;

	// Apply the scale vector [s0, s1 ... sn] and [l0, l1, ... ln]
	// Error currently seems to be located in the latter half of the genes
	for (int i = 0; i < individuals; i++) {
		for (int j = 0; j < (int)floorf(genes/ 2); j += 2) { // I think this causes the error
            U1 = ((float) gen_mt_rand(mt_rand) / UINT32_MAX);
            U2 = ((float) gen_mt_rand(mt_rand) / UINT32_MAX);

            scale = 1/pop_param.sigma * (pop_param.upper[i] - pop_param.lower[i])/2;
            loc = (pop_param.upper[i] + pop_param.lower[i]) / 2;

			z1 = sqrtf(-2 * logf(U1)) * cosf(2 * PI * U2);
			z2 = sqrtf(-2 * logf(U1)) * sinf(2 * PI * U2); // Maybe the sin messes up?


			res1 = float2bin((z1 * scale) + loc, pop_param.lower[i], pop_param.upper[i]);
			res2 = float2bin((z2 * scale) + loc, pop_param.lower[i], pop_param.upper[i]);
			result[i][j] = res1;
			result[i][j + 1] = res2;
		}
        if (extra) { // Add the last one
            result[i][genes - 1] = float2bin((sqrtf(-2 * logf(gen_mt_rand(mt_rand))) * cosf(2 * PI * gen_mt_rand(mt_rand)) * scale) + loc, pop_param.lower[i], pop_param.upper[i]);
        }
	}
}

void cauchy_bit_pop(int** result, int individuals, int genes, population_param_t pop_param, mt_rand_t* mt_rand) {
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

	double cauchyfloat;
	double scaledcauchy;

	for (int i = 0; i < individuals; i++) {
		for (int j = 0; j < genes; j++) {
			scale = 1 / pop_param.sigma * (pop_param.upper[i] - pop_param.lower[i]) / 2;
			loc = (pop_param.upper[i] + pop_param.lower[i]) / 2;
			
			cauchyfloat = cauchy((gen_mt_rand(mt_rand) << 32) | gen_mt_rand(mt_rand), 0, 1);
            scaledcauchy = (cauchyfloat * scale) + loc;
			
			result[i][j] = float2bin(scaledcauchy, pop_param.lower[i], pop_param.upper[i]);
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


	gene_pool->flatten_result_set = malloc(gene_pool->individuals * sizeof(double));
	gene_pool->pop_param_bin = (int**)malloc(gene_pool->individuals * sizeof(int*));
	gene_pool->pop_param_bin_cross_buffer = (int**)malloc(gene_pool->individuals * sizeof(int*));
	gene_pool->pop_param_double = malloc(gene_pool->individuals * sizeof(double*));
	gene_pool->pop_result_set = malloc(gene_pool->individuals * sizeof(double));
	gene_pool->selected_indexes = malloc(gene_pool->individuals * sizeof(int));
	gene_pool->sorted_indexes = malloc(gene_pool->individuals * sizeof(int));
	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->pop_param_bin[i] = (int*)malloc(gene_pool->genes * sizeof(int));
		gene_pool->pop_param_bin_cross_buffer[i] = (int*)malloc(gene_pool->genes * sizeof(int));
		gene_pool->pop_param_double[i] = (double*)malloc(gene_pool->genes * sizeof(double));
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

void fill_individual(gene_pool_t* gene_pool, int individual, mt_rand_t* mt_rand) {
	bitpop32(gene_pool->genes, gene_pool->pop_param_bin[individual], mt_rand);
}

void fill_pop(gene_pool_t* gene_pool, population_param_t pop_param) {
	mt_rand_t *mt_rand = getThreadRand();

	if (pop_param.sampling_type == pop_uniform)
		for (int i = 0; i < gene_pool->individuals; i++) {
			fill_individual(gene_pool, i, mt_rand);
		}
	else if (pop_param.sampling_type == pop_normal) {
		normal_bit_pop_boxmuller(gene_pool->pop_param_bin, gene_pool->individuals, gene_pool->genes, pop_param, mt_rand);
	}
	else if (pop_param.sampling_type == pop_cauchy) {
		cauchy_bit_pop(gene_pool->pop_param_bin, gene_pool->individuals, gene_pool->genes, pop_param, mt_rand);
	}
}