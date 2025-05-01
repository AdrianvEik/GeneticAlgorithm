
#include "Helper.h"

//void indexed_merge_sort(double* arr, int* indices, int size) {
//	if (size > 1) {
//		int mid = size / 2;
//		int* L_indices = (int*)malloc(mid * sizeof(int));
//		int* R_indices = (int*)malloc((size - mid) * sizeof(int));
//
//
//		for (int i = 0; i < mid; i++) {
//			L_indices[i] = indices[i];
//		}
//		for (int i = mid; i < size; i++) {
//			R_indices[i - mid] = indices[i];
//		}
//
//		indexed_inv_merge_sort(arr, L_indices, mid);
//		indexed_inv_merge_sort(arr, R_indices, size - mid);
//
//		int i = 0;
//		int j = 0;
//		int k = 0;
//
//		while (i < mid && j < size - mid) {
//			if (arr[indices[i]] < arr[indices[j]]) {
//				indices[k] = L_indices[i];
//				i++;
//			}
//			else {
//				indices[k] = R_indices[j];
//				j++;
//			}
//			k++;
//		}
//
//		while (i < mid) {
//			indices[k] = L_indices[i];
//			i++;
//			k++;
//		}
//
//		while (j < size - mid) {
//			indices[k] = R_indices[j];
//			j++;
//			k++;
//		}
//
//
//		free(L_indices);
//		free(R_indices);
//	}
//}
//
//void indexed_inv_merge_sort(double* arr, int* indices, int size) {
//	if (size > 1) {
//		int mid = size / 2;
//		int* L_indices = (int*)malloc(mid * sizeof(int));
//		int* R_indices = (int*)malloc((size - mid) * sizeof(int));
//		double* L = (double*)malloc(mid * sizeof(double));
//		double* R = (double*)malloc((size - mid) * sizeof(double));
//		if (L_indices == NULL || R_indices == NULL || L == NULL || R == NULL) {
//			printf("Memory allocation failed");
//			exit(255);
//		}
//
//		for (int i = 0; i < mid; i++) {
//			L[i] = arr[i];
//			L_indices[i] = indices[i];
//		}
//		for (int i = mid; i < size; i++) {
//			R[i - mid] = arr[i];
//			R_indices[i - mid] = indices[i];
//		}
//
//		indexed_inv_merge_sort(L, L_indices, mid);
//		indexed_inv_merge_sort(R, R_indices, size - mid);
//
//		int i = 0;
//		int j = 0;
//		int k = 0;
//
//		while (i < mid && j < size - mid) {
//			if (L[i] > R[j]) {
//				arr[k] = L[i];
//				indices[k] = L_indices[i];
//				i++;
//			}
//			else {
//				arr[k] = R[j];
//				indices[k] = R_indices[j];
//				j++;
//			}
//			k++;
//		}
//
//		while (i < mid) {
//			arr[k] = L[i];
//			indices[k] = L_indices[i];
//			i++;
//			k++;
//		}
//
//		while (j < size - mid) {
//			arr[k] = R[j];
//			indices[k] = R_indices[j];
//			j++;
//			k++;
//		}
//
//		free(L);
//		free(R);
//		free(L_indices);
//		free(R_indices);
//	}
//}

