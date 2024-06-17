
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
			// result[i][j] = (double) temp[i][j] / (pow(2, bitsize - 1)) * factor + bias;
			
			result[i][j] = ((double)valarr[i][j] * (upper[j] - lower[j]) + lower[j]) / max_int;
			//result[i][j] = (double)temp * factor / (pow(2, 8 * sizeof(int) - 1)) + bias;
		}
	}
}

//void int2ndbit32(double** valarr, int genes, int individuals,
//	double* lower, double* upper, int** result) {
//
//	/*
//	Convert an array of integers to an array of bitarrays
//
//	:param valarr: The array of integers to be converted to bitarrays (a)
//	:type valarr: array of doubles (double **)
//
//	:param bitsize: The size of the bitarrays
//	:type bitsize: int
//
//	:param genes: The number of genes in the bitarrays (n = genes * bitsize; n = a * bitsize)
//	:type genes: int
//
//	:param individuals: the number of individuals in the bitarrays (m = individuals; m = a)
//	:type individuals: int
//
//	:param result: The array of bitarrays to be filled with the converted values (m x n)
//	:type result: array of ints (int **)
//
//	:param factor: The factor of the uniform distribution.
//	:type factor: double
//
//	:param bias: The bias of the uniform distribution.
//	:type bias: double
//
//	:return: void
//	:rtype: void
//	*/
//
//	// normalise the values and apply the factor and bias and cast to integers
//	int temp;
//	int max_int;
//	max_int = pow(2, 8 * sizeof(int) - 1);
//	for (int i = 0; i < individuals; i++) {
//		for (int j = 0; j < genes; j++) {
//			temp = (int)round((valarr[i][j] - lower[j]) * max_int / (upper[j] - lower[j]));
//			if (temp < 0) {
//				result[i][j] = ~(temp - 1) | 0x80000000; // bitflip and subtract 1
//			}
//			else {
//				result[i][j] = temp;
//			}
//		}
//	}
//
//}

void ndbit2int(int** valarr, int bitsize, int genes, int individuals,
	double factor, double bias, double** result) {
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

	// temp int array to store the values
	int** temp = (int**)malloc(individuals * sizeof(int*));
	if (temp == NULL) {
		printf("Memory allocation failed");
		exit(255);
	}

	for (int i = 0; i < individuals; i++) {
		temp[i] = (int*)malloc(genes * sizeof(int));
		if (temp[i] == NULL) {
			printf("Memory allocation failed");
			exit(255);
		}
	}


	// convert the values to integers
	binmat2intmat(valarr, bitsize, genes, individuals, temp);

	// normalise the values and apply the factor and bias
	for (int i = 0; i < individuals; i++) {
		for (int j = 0; j < genes; j++) {
			result[i][j] = (double)temp[i][j] / (pow(2, bitsize - 1)) * factor + bias;
		}
	}



	// free the temp array
	for (int i = 0; i < individuals; i++) {
		free(temp[i]);
	}
	free(temp);
}

void int2ndbit(double** valarr, int bitsize, int genes, int individuals,
	double factor, double bias, int** result) {

	/*
	Convert an array of integers to an array of bitarrays

	:param valarr: The array of integers to be converted to bitarrays (a)
	:type valarr: array of doubles (double **)

	:param bitsize: The size of the bitarrays
	:type bitsize: int

	:param genes: The number of genes in the bitarrays (n = genes * bitsize; n = a * bitsize)
	:type genes: int

	:param individuals: the number of individuals in the bitarrays (m = individuals; m = a)
	:type individuals: int

	:param result: The array of bitarrays to be filled with the converted values (m x n)
	:type result: array of ints (int **)

	:param factor: The factor of the uniform distribution.
	:type factor: double

	:param bias: The bias of the uniform distribution.
	:type bias: double


	:return: void
	:rtype: void
	*/

	// create a copy of valarr for integer conversion
	int** copyvalarr = (int**)malloc(individuals * sizeof(int*));
	if (copyvalarr == NULL) {
		printf("Memory allocation failed");
		exit(255);
	}

	for (int i = 0; i < individuals; i++) {
		copyvalarr[i] = (int*)malloc(genes * sizeof(int));
		if (copyvalarr[i] == NULL) {
			printf("Memory allocation failed");
			exit(255);
		}
	}

	// normalise the values and apply the factor and bias and cast to integers
	for (int i = 0; i < individuals; i++) {
		for (int j = 0; j < genes; j++) {
			copyvalarr[i][j] = (int)round((valarr[i][j] - bias) / factor * pow(2, bitsize - 1));
		}
	}



	// convert the values to bitarrays
	intmat2binmat(copyvalarr, bitsize, genes, individuals, result);

	// free the copyvalarr array
	for (int i = 0; i < individuals; i++) {
		free(copyvalarr[i]);
	}

	free(copyvalarr);
}

void int2bin(int value, int bitsize, int* result) {
	/*
	Convert an integer to a bitarray

	:param value: The integer to be converted to a bitarray
	:type value: int

	:param bitsize: is the size of the bitarray
	:type bitsize: int

	:param result: is the bitarray to be filled with the converted values
	:type result: array of bytes
	*/

	if (value < 0) {
		result[0] = 1;
		value = value * -1;
	}
	else {
		result[0] = 0;
	}

	// convert the value to a bitarray
	for (int i = 1; i < bitsize; i++) {
		result[i] = value % 2;
		value = value / 2;
	}
}

void intarr2binarr(int* valarr, int bitsize, int genes, int* result) {
	/*

	Convert an array of integers to an array of bitarrays

	:param valarr: The array of integers to be converted to bitarrays (a)
	:type valarr: array of ints (int *)

	:param bitsize: The size of the bitarrays
	:type bitsize: int

	:param genes: The number of genes in the bitarrays (n = genes / bitsize; n = a / bitsize)
	:type genes: int

	:param result: The array of bitarrays to be filled with the converted values (n * bitsize)
	:type result: array of ints (int *)

	:return: void
	*/

	// convert the values to bitarrays
	for (int i = 0; i < genes; i++) {
		int2bin(valarr[i], bitsize, &result[i * bitsize]);
	}

}


void intmat2binmat(int** valmat, int bitsize, int genes, int individuals, int** result) {
	/*

	Convert a matrix of integers to a matrix of bitarrays (a x b) (individuals x genes)

	:param valmat: The matrix of integers to be converted to bitarrays (a x b) (individuals x genes)
	:type valmat: array of ints (int **)

	:param bitsize: The size of the bitarrays
	:type bitsize: int

	:param genes: The number of genes in the bitarrays (n = genes * bitsize; n = b * bitsize)
	:type genes: int

	:param individuals: The number of individuals in the bitarrays (m = individuals; m = a)
	:type individuals: int

	:param result: The matrix of bitarrays to be filled with the converted values (m x n)
	:type result: array of ints (int **)

	*/

	// convert the values to bitarrays
	for (int i = 0; i < individuals; i++) {
		intarr2binarr(valmat[i], bitsize, genes, result[i]);
	}
}

int bin2int(int* value, int bitsize) {

	/*

	Convert a bitarray to an integer

	:param value: The bitarray to be converted to an integer
	:type value: array of ints (int *)

	:param bitsize: The size of the bitarray
	:type bitsize: int

	:return: The integer value of the bitarray
	:rtype: int

	*/

	// convert the bitarray to an integer

	int sign = 1;
	int res = 0;


	if (value[0] == 1) {
		sign = -1;
	}
	else {
		sign = 1;
	}

	for (int i = 1; i < bitsize; i++) {
		res += value[i] * (int)pow(2, i - 1);
	}
	res = res * sign;

	return res;
}


void binarr2intarr(int* value, int bitsize, int genes, int* result) {

	/*
	Convert an array of bitarrays to an array of integers

	:param valarr: The array of bitarrays to be converted to integers (a)
	:type valarr: array of ints (int *)

	:param bitsize: The size of the bitarrays
	:type bitsize: int

	:param genes: The number of genes in the bitarrays (n = genes / bitsize; n = a / bitsize)
	:type genes: int

	:param result: The array of integers to be filled with the converted values (n)
	:type result: array of ints (int *)

	:return: void
	*/


	// convert the values to integers
	for (int i = 0; i < genes; i++) {
		result[i] = bin2int(&value[i * bitsize], bitsize);
	}
}

void binmat2intmat(int** valmat, int bitsize, int genes, int individuals, int** result) {

	/*
	Convert a matrix of bitarrays to a matrix of integers (a x b) (individuals x genes)

	:param valmat: The matrix of bitarrays to be converted to integers (a x b) (individuals x genes)
	:type valmat: array of ints (int **)

	:param bitsize: The size of the bitarrays
	:type bitsize: int

	:param genes: The number of genes in the bitarrays (n = genes * bitsize; n = b * bitsize)
	:type genes: int

	:param individuals: The number of individuals in the bitarrays (m = individuals; m = a)
	:type individuals: int

	:param result: The matrix of integers to be filled with the converted values (m x n)
	:type result: array of ints (int **)
	*/

	for (int i = 0; i < individuals; i++) {
		binarr2intarr(valmat[i], bitsize, genes, result[i]);
	}

}

void printMatrix(int** matrix, int rows, int cols) {

	/*

	Print a matrix of integers

	:param matrix: The matrix to be printed
	:type matrix: array of ints (int **)

	:param rows: The number of rows in the matrix
	:type rows: int

	:param cols: The number of columns in the matrix
	:type cols: int

	:return: void

	*/

	printf("cols: %d\n", cols);
	printf("rows: %d\n", rows);

	printf("[");
	for (int i = 0; i < rows; i++) {
		printf("[");
		for (int j = 0; j < cols; j++) {
			printf("%d", matrix[i][j]);
			if (j < cols - 1) {
				printf(", ");
			}
		}
		printf("]");
		if (i < rows - 1) {
			printf(", \n");
		}
	}
	printf("]\n");
}

void printfMatrix(double** matrix, int rows, int cols, int precision) {

	/*
	Print a matrix of doubles

	:param matrix: The matrix to be printed
	:type matrix: array of doubles (double **)

	:param rows: The number of rows in the matrix
	:type rows: int

	:param cols: The number of columns in the matrix
	:type cols: int

	:param precision: The number of decimals to be printed
	:type precision: int

	:return: void

	*/


	printf("cols: %d\n", cols);
	printf("rows: %d\n", rows);

	printf("[");
	for (int i = 0; i < rows; i++) {
		printf("[");
		for (int j = 0; j < cols; j++) {
			printf("%.*f", precision, matrix[i][j]);
			if (j < cols - 1) {
				printf(", ");
			}
		}
		printf("]");
		if (i < rows - 1) {
			printf(", \n");
		}
	}
	printf("]\n");
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

void uniform_random(int m, int n, int lower, int upper, int** result) {
	/*
	Create a matrix filled with uniformly distributed integers.

	:param m: Amount of rows
	:type m: int
	:param n: Amount of cols
	:type n: int

	:param lower: Lower bound of the distribution
	:type lower: int
	:param upper: Upper bound of the distribution
	:type upper: int

	:param result: Result matrix to which the distibutution is written to
	:type result: **int (m x n)

	:return: void
	*/

	// the range in which the numbers can be generated
	unsigned int nRange = (unsigned int)(upper - lower);
	// The amount of bits generated for a number
	unsigned int nRangeBits = (unsigned int)ceil(log2((double)(nRange)));

	unsigned int nRand; // random number


	// for the matrix write uniformly distributed numbers
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			do {
				nRand = 0;
				for (unsigned int k = 0; k < nRangeBits; k++) {
					nRand = (nRand << 1) | (rand() & 1); // lshift and rand or 1
				}
			} while (nRand >= nRange);
			result[i][j] = (int)(nRand + lower);
		}
	}


}

double gaussian(double x, double mu, double sigma) {
	/*
	Calculate the gaussian of x

	x is the input
	mu is the mean
	sigma is the standard deviation
	*/

	double result = (1 / (sigma * sqrt(2 * PI))) * exp(-pow(x - mu, 2) / (2 * pow(sigma, 2)));

	return result;
}

double cauchy(double x, double mu, double sigma) {
	/*
	Calculate the cauchy of x

	x is the input
	mu is the mean
	sigma is the standard deviation
	*/

	double result = (1 / PI) * (sigma / (pow(x - mu, 2) + pow(sigma, 2)));

	return result;
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
		double randnum = ((double)random_intXOR32()) * normaliser;

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

int intXOR32_seed = 0;
int intXOR32_generated = 0;

unsigned int random_int32() {
	srand((unsigned int)time(0));
	return (rand() << 30) | (rand() << 15) | (rand());
}

unsigned int random_intXOR32() {
	if (intXOR32_generated > 100 || intXOR32_seed == 0) {
		seed_intXOR32();
		intXOR32_generated = 0;
	}
	int a = intXOR32_seed;
	intXOR32_seed = intXORshift32(a);
	intXOR32_generated++;
	return a;
}

void seed_intXOR32() {
	if (intXOR32_seed == 0) {
		intXOR32_seed = random_int32();
		printf("seed: %d\n", intXOR32_seed);
	}
}

unsigned int intXORshift32(unsigned int a) {
	a ^= a << 13;
	a ^= a >> 17;
	a ^= a << 5;
	return a;
}

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


//void convert_int32_to_binary(int** valarr, int genes, int individuals,
//	double factor, double bias) {
//
//
//	double** temp = (double**)malloc(individuals * sizeof(double*));
//	if (temp == NULL) {
//		printf("Memory allocation failed");
//		exit(255);
//	}
//
//	for (int i = 0; i < individuals; i++) {
//		temp[i] = (double*)malloc(genes * sizeof(double) * 8 * sizeof(int));
//		if (temp[i] == NULL) {
//			printf("Memory allocation failed");
//			exit(255);
//		}
//	}
//
//	ndbit2int32(valarr, genes, individuals, factor, bias, temp);
//	int2ndbit(temp, 8 * sizeof(int), genes, individuals, factor, bias, valarr);
//
//	for (int i = 0; i < individuals; i++) {
//		free(temp[i]);
//	}
//
//	free(temp);
//}
//
//void convert_binary_to_int32(int** valarr, int genes, int individuals,
//	double factor, double bias) {
//
//	double** temp = (double**)malloc(individuals * sizeof(double*));
//	if (temp == NULL) {
//		printf("Memory allocation failed");
//		exit(255);
//	}
//
//	for (int i = 0; i < individuals; i++) {
//		temp[i] = (double*)malloc(genes * sizeof(double) * 8 * sizeof(int));
//		if (temp[i] == NULL) {
//			printf("Memory allocation failed");
//			exit(255);
//		}
//	}
//
//
//	ndbit2int(valarr, 8 * sizeof(int), genes, individuals, factor, bias, temp);
//	int2ndbit32(temp, genes, individuals, factor, bias, valarr);
//
//	for (int i = 0; i < individuals; i++) {
//		free(temp[i]);
//	}
//
//	free(temp);
//}