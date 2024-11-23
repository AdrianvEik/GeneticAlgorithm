
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "stdint.h"
#include "time.h"

#include "Helper.h"
#undef PI
#define PI   3.14159265358979323846264338327950288419716939937510f


void ndbit2int32(unsigned int** valarr, int genes, int individuals,
	double* lower, double* upper, double** result) {
	/*
	Convert an array of bitarrays to an array of doubles

	:param valarr: The array of binary data to be converted to doubles (a x b) (individuals x (bitsize * genes))
	:type valarr: array of doubles (double **)

	:param bitsize: The size of the bitarrays
	:type bitsize: int

	:param genes: The number of genes in the bitarrays (n = length of a row / bitsize; n = a / bitsize)
	:type genes: int

	:param individuals: the number of individuals in the bitarrays (m = individuals; m = a)
	:type individuals: int

	:param result: The array of doubles to be filled with the converted values (m x n)
	:type result: array of ints (double **)

	:param factor: The factor of the uniform distribution.
	:type factor: double

	:param bias: The bias of the uniform distribution.
	:type bias: double

	:return: void
	:rtype: void
	*/

	double max_int;
	max_int = pow(2, 8 * sizeof(int));

	for (int i = 0; i < individuals; i++) {
		for (int j = 0; j < genes; j++) {			
			result[i][j] = (double)(valarr[i][j] * (upper[j] - lower[j])) / max_int + lower[j];
		}
	}
}


void sigmoid(double* x, double* result, int size) {
	/*
	Calculate the sigmoid of x

	x is the input
	result is the output
	*/

	for (int i = 0; i < size; i++) {
		result[i] = 1 / (1 + exp(-x[i]));
	}
}

void sigmoid_derivative(double* x, double* result, int size) {
	/*
	Calculate the derivative of the sigmoid of x

	x is the input
	result is the output
	*/

	for (int i = 0; i < size; i++) {
		result[i] = x[i] * (1 - x[i]);
	}
}

void sigmoid2(double* x, double a, double b, double c, double d, double Q, double nu, double* result, int size) {

	/*
	Calculate the sigmoid of x

	x is the input
	result is the output
	*/

	for (int i = 0; i < size; i++) {
		result[i] = a + (b - a) / (1 + Q * pow(exp(-c * (x[i] - d)), (1 / nu)));
	}
}


double gaussian(double x, double mu, double sigma) {
	/*
	Calculate the gaussian of x

	x is the input
	mu is the mean
	sigma is the standard deviation
	*/

	return (1 / (sigma * sqrt(2 * PI))) * exp(-pow(x - mu, 2) / (2 * pow(sigma, 2)));
}

double cauchy(double x, double mu, double sigma) {
	/*
	Calculate the cauchy of x

	x is the input
	mu is the mean
	sigma is the standard deviation
	*/

	return (1 / PI) * (sigma / (pow(x - mu, 2) + pow(sigma, 2)));
}

void roulette_wheel(double* probabilities, int size, int ressize, int* result) {

	/*
	Roulette wheel selection of an index based on probabilities

	:param probabilities: The probabilities of the indices
	:type probabilities: array of doubles (double *)

	:param size: The size of the probabilities array
	:type size: int

	:param ressize: The size of the result array (amount of indices to be selected)
	:type ressize: int

	:param result: The index selected
	:type result: array of ints (int *)

	*/

	// calculate the cumulative sum of the probabilities
	double* cumsum = (double*)malloc(size * sizeof(double));
	if (cumsum == NULL) {
		printf("Memory allocation failed");
		exit(255);
	}
	cumsum[0] = probabilities[0];

	for (int i = 1; i < size; i++) {
		cumsum[i] = cumsum[i - 1] + probabilities[i];
	}

	double normaliser = cumsum[size - 1] / (double)0xffffffff;

	// generate random numbers and select the indices
	for (int i = 0; i < ressize; i++) {
		double randnum = ((double) gen_mt_rand()) * normaliser;

		for (int j = 0; j < size; j++) {
			if (randnum < cumsum[j]) {
				result[i] = j;
				break;
			}
		}
	}

	// free the arrays
	free(cumsum);
}

//int intXOR32_seed = 0;
//int intXOR32_generated = 0;

//unsigned int random_int32() {
//	srand((unsigned int)time(0));
//	return (rand() << 30) | (rand() << 15) | (rand());
//}
//
//unsigned int random_intXOR32() {
//	if (intXOR32_generated > 100 || intXOR32_seed == 0) {
//		seed_intXOR32();
//		intXOR32_generated = 0;
//	}
//	int a = intXOR32_seed;
//	intXOR32_seed = intXORshift32(a);
//	intXOR32_generated++;
//	return a;
//}
//
//void seed_intXOR32() {
//	if (intXOR32_seed == 0) {
//		intXOR32_seed = random_int32();
//		printf("seed: %d\n", intXOR32_seed);
//	}
//}
//
//
//unsigned int intXORshift32(unsigned int a) {
//	a ^= a << 13;
//	a ^= a >> 17;
//	a ^= a << 5;
//	return a;
//}

void indexed_bubble_sort(double* arr, int* indices, int size) {
	int swapped = 1;
	int temp_idx;

	for (int i = 0; i < size && swapped; i++) {
		swapped = 0;
		for (int j = 0; j < size - i - 1; j++) {
			if (arr[indices[j]] > arr[indices[j + 1]]) {
				temp_idx = indices[j];
				indices[j] = indices[j + 1];
				indices[j + 1] = temp_idx;
				swapped = 1;
			}
		}
	}
}

void indexed_merge_sort(double* arr, int* indices, int size) {
	if (size > 1) {
		int mid = size / 2;
		int* L_indices = (int*)malloc(mid * sizeof(int));
		int* R_indices = (int*)malloc((size - mid) * sizeof(int));


		for (int i = 0; i < mid; i++) {
			L_indices[i] = indices[i];
		}
		for (int i = mid; i < size; i++) {
			R_indices[i - mid] = indices[i];
		}

		indexed_inv_merge_sort(arr, L_indices, mid);
		indexed_inv_merge_sort(arr, R_indices, size - mid);

		int i = 0;
		int j = 0;
		int k = 0;

		while (i < mid && j < size - mid) {
			if (arr[indices[i]] < arr[indices[j]]) {
				indices[k] = L_indices[i];
				i++;
			}
			else {
				indices[k] = R_indices[j];
				j++;
			}
			k++;
		}

		while (i < mid) {
			indices[k] = L_indices[i];
			i++;
			k++;
		}

		while (j < size - mid) {
			indices[k] = R_indices[j];
			j++;
			k++;
		}


		free(L_indices);
		free(R_indices);
	}
}

void indexed_inv_merge_sort(double* arr, int* indices, int size) {
	if (size > 1) {
		int mid = size / 2;
		int* L_indices = (int*)malloc(mid * sizeof(int));
		int* R_indices = (int*)malloc((size - mid) * sizeof(int));
		double* L = (double*)malloc(mid * sizeof(double));
		double* R = (double*)malloc((size - mid) * sizeof(double));
		if (L_indices == NULL || R_indices == NULL || L == NULL || R == NULL) {
			printf("Memory allocation failed");
			exit(255);
		}

		for (int i = 0; i < mid; i++) {
			L[i] = arr[i];
			L_indices[i] = indices[i];
		}
		for (int i = mid; i < size; i++) {
			R[i - mid] = arr[i];
			R_indices[i - mid] = indices[i];
		}

		indexed_inv_merge_sort(L, L_indices, mid);
		indexed_inv_merge_sort(R, R_indices, size - mid);

		int i = 0;
		int j = 0;
		int k = 0;

		while (i < mid && j < size - mid) {
			if (L[i] > R[j]) {
				arr[k] = L[i];
				indices[k] = L_indices[i];
				i++;
			}
			else {
				arr[k] = R[j];
				indices[k] = R_indices[j];
				j++;
			}
			k++;
		}

		while (i < mid) {
			arr[k] = L[i];
			indices[k] = L_indices[i];
			i++;
			k++;
		}

		while (j < size - mid) {
			arr[k] = R[j];
			indices[k] = R_indices[j];
			j++;
			k++;
		}

		free(L);
		free(R);
		free(L_indices);
		free(R_indices);
	}
}