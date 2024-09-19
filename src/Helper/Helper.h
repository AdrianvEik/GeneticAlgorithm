

#ifndef _HELPER_H
#define _HELPER_H



// Convesrion functions
void ndbit2int32(unsigned int** valarr, int genes, int individuals,
	double* lower, double* upper, double** result);
//void int2ndbit32(double** valarr, int genes, int individuals,
//	double factor, double bias, int** result);
void ndbit2int(int** valarr, int bitsize, int genes, int individuals,
	double factor, double bias, double** result);
void int2ndbit(double** valarr, int bitsize, int genes, int individuals,
	double factor, double bias, int** result);

// conversion helper functions
void int2bin(int value, int bitsize, int* result);
void intarr2binarr(int* valarr, int bitsize, int size, int* result);
void intmat2binmat(int** valmat, int bitsize, int genes, int individuals, int** result);
int bin2int(int* value, int bitsize);
void binarr2intarr(int* value, int bitsize, int genes, int* result);
void binmat2intmat(int** valmat, int bitsize, int genes, int individuals, int** result);

// Printing functions
void printMatrix(int** matrix, int rows, int cols);
void printfMatrix(double** matrix, int rows, int cols, int precision);

// Mathemathical functions
void sigmoid(double* x, double* result, int size);
void sigmoid_derivative(double* x, double* result, int size);
void sigmoid2(double* x, double a, double b, double c, double d, double Q, double nu, double* result, int size);
void uniform_random(int m, int n, int lower, int upper, int** result);
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
