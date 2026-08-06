
#ifndef SORT_H
#define SORT_H

#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <float.h>

#include "Helper.h"

void indexed_bubble_sort(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size);

void indexed_merge_sort(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size);

void indexed_bitonic_sort_1v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size);
void indexed_bitonic_sort_2v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size);
void indexed_bitonic_sort_4v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size);
void indexed_bitonic_sort_8v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size);
void indexed_bitonic_sort_16v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size);

#endif // ! SORT_H


