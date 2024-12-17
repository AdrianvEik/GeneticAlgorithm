#include "rng.h"
#include "../Helper/multiprocessing.h"

#ifndef _HELPER_H
#define _HELPER_H


// Convesrion functions
void ndbit2int32(unsigned int** valarr, int genes, int individuals,
	double* lower, double* upper, double** result);



// Mathemathical functions
void sigmoid(double* x, double* result, int size);
void sigmoid_derivative(double* x, double* result, int size);
void sigmoid2(double* x, double a, double b, double c, double d, double Q, double nu, double* result, int size);
double gaussian(double x, double mu, double sigma);
double cauchy(double x, double mu, double sigma);

// Roulette wheel selection
void roulette_wheel(double* probabilities, int size, int ressize, int* result);

// sorting functions
void indexed_bubble_sort(double* arr, int* indices, int size);
void indexed_merge_sort(double* arr, int* indices, int size);
void indexed_inv_merge_sort(double* arr, int* indices, int size);


// random 32 bit integer in binary
//unsigned int random_int32();
//void seed_intXOR32();
//unsigned int random_intXOR32();
//unsigned int intXORshift32(unsigned int a);

// usefull for debugging and eventual conversion to numpy arrays
//void convert_int32_to_binary(int** valarr, int genes, int individuals,
//	double factor, double bias);
//void convert_binary_to_int32(int** valarr, int genes, int individuals,
//	double factor, double bias);

#endif
