#include "process.h"

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
//inline __AVXd AVX_512_bitonic_sort_1v(__AVXd input) {
//
//	__AVXi idxNoNeigh = mm512 set epi64(6, 7, 4, 5, 2, 3, 0, 1);
//	__AVXd permNeigh = mm512 permutexvar pd(idxNoNeigh, input);
//	__AVXd permNeighMin = mm512min pd(permNeigh, input);
//	__AVXd permNeighMax = mm512max pd(permNeigh, input);
//	input = mm512maskmov pd(permNeighMin, 0xAA, permNeighMax);
//
//
//	__AVXi idxNoNeigh = mm512 set epi64(4, 5, 6, 7, 0, 1, 2, 3);
//	__AVXd permNeigh = mm512 permutexvar pd(idxNoNeigh, input);
//	__AVXd permNeighMin = mm512min pd(permNeigh, input);
//	__AVXd permNeighMax = mm512max pd(permNeigh, input);
//	input = mm512maskmov pd(permNeighMin, 0xCC, permNeighMax);
//
//
//	__AVXi idxNoNeigh = mm512 set epi64(6, 7, 4, 5, 2, 3, 0, 1);
//	__AVXd permNeigh = mm512 permutexvar pd(idxNoNeigh, input);
//	__AVXd permNeighMin = mm512min pd(permNeigh, input);
//	__AVXd permNeighMax = mm512max pd(permNeigh, input);
//	input = mm512maskmov pd(permNeighMin, 0xAA, permNeighMax);
//
//
//	__AVXi idxNoNeigh = mm512 set epi64(0, 1, 2, 3, 4, 5, 6, 7);
//	__AVXd permNeigh = mm512 permutexvar pd(idxNoNeigh, input);
//	__AVXd permNeighMin = mm512min pd(permNeigh, input);
//	__AVXd permNeighMax = mm512max pd(permNeigh, input);
//	input = mm512maskmov pd(permNeighMin, 0xF0, permNeighMax);
//
//
//	__AVXi idxNoNeigh = mm512 set epi64(5, 4, 7, 6, 1, 0, 3, 2);
//	__AVXd permNeigh = mm512 permutexvar pd(idxNoNeigh, input);
//	__AVXd permNeighMin = mm512min pd(permNeigh, input);
//	__AVXd permNeighMax = mm512max pd(permNeigh, input);
//	input = mm512maskmov pd(permNeighMin, 0xCC, permNeighMax);
//
//
//	__AVXi idxNoNeigh = mm512 set epi64(6, 7, 4, 5, 2, 3, 0, 1);
//	__AVXd permNeigh = mm512 permutexvar pd(idxNoNeigh, input);
//	__AVXd permNeighMin = mm512min pd(permNeigh, input);
//	__AVXd permNeighMax = mm512max pd(permNeigh, input);
//	input = mm512maskmov pd(permNeighMin, 0xAA, permNeighMax);
//
//
//	return input;
//}
//

inline __m256i bitonic_sort_1v(__m512d input, __m256i index) { // todo update int to uint for index
	//__m256i index = _mm256_loadu_epi32(indices);
    //__m512d input = _mm512_load_pd(arr);
    __m512d work;
	__m256i work_idx;
	__mmask8 cmp_res;

	work = _mm512_permutexvar_pd(_mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1), input);
    cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, input), 0xAA);
    input = _mm512_mask_mov_pd(input, cmp_res, work);
	work_idx = _mm256_mask_mov_epi32(index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), index));
    index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(4, 5, 6, 7, 0, 1, 2, 3), input);
    cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, input), 0xCC);
    input = _mm512_mask_mov_pd(input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(4, 5, 6, 7, 0, 1, 2, 3), index));
    index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1), input);
    cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, input), 0xAA);
    input = _mm512_mask_mov_pd(input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), index));
    index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7), input);
    cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, input), 0xF0);
    input = _mm512_mask_mov_pd(input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7), index));
    index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2), input);
    cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, input), 0xCC);
    input = _mm512_mask_mov_pd(input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(5, 4, 7, 6, 1, 0, 3, 2), index));
    index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1), input);
    cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, input), 0xAA);
    input = _mm512_mask_mov_pd(input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), index));
    index = work_idx;

    //_mm512_store_pd(arr, input); // store sorted values back to array??
    //_mm256_storeu_epi32(indices, index);
    // return set indexes
    return index;
}

void bottom_up_merge(double* arr, int* indices, int* temp_workspace, int lo, int hi, int size) {
    // merge blocks of size 8 to 16 , 16 to 32, 32 to 64, etc


}

void indexed_bitonic_sort(double* arr, uint32_t* indices, int* temp_workspace, int size ) {

	for (int i = 0; i < size / 8; i++) {
		__m256i index = _mm256_loadu_epi32(&indices[i * 8]);
		__m512d input = _mm512_loadu_pd(&arr[i * 8]);
		index = bitonic_sort_1v(input, index);
		_mm256_storeu_epi32(&indices[i * 8], index);
	}

	// remainder
	if (size % 8 != 0) {
		int rem = size % 8;
		double temp_arr[8] = { DBL_MAX };
		uint32_t temp_idx[8] = { UINT32_MAX };
		for (int i = 0; i < rem; i++) {
			temp_arr[i] = arr[(size / 8) * 8 + i];
			temp_idx[i] = indices[(size / 8) * 8 + i];
		}
		__m256i index = _mm256_loadu_epi32(temp_idx);
		__m512d input = _mm512_loadu_pd(temp_arr);
		index = bitonic_sort_1v(input, index);
		_mm256_storeu_epi32(temp_idx, index);
		uint32_t skip_idx = 0;
		for (int i = 0; i < rem; i++) {
            if (temp_idx[i] == UINT32_MAX) {
                skip_idx++;
				continue;
			}
			indices[(size / 8) * 8 + i] = temp_idx[i+skip_idx];
		}
    }

	int i, mid, j, lo, hi, k, iter = 0;

	for (iter = 8; iter < size; iter *= 2) {
		for (lo = 0; lo < size - iter; lo += iter * 2) {
			mid = lo + iter;
			hi = (lo + iter * 2 < size) ? (lo + iter * 2) : size;
			i = lo;
			j = mid;
			k = lo;
			while (i < mid && j < hi) {
				if (arr[indices[i]] <= arr[indices[j]]) {
					temp_workspace[k++] = indices[i++];
				}
				else {
					temp_workspace[k++] = indices[j++];
				}
			}
			while (i < mid) {
				temp_workspace[k++] = indices[i++];
			}
			while (j < hi) {
				temp_workspace[k++] = indices[j++];
			}
			for (int m = lo; m < hi; m++) {
				indices[m] = temp_workspace[m];
			}
        }
	}
	// TODO handle remaining elements <8
    // TODO handle case when size <8

	//__m256i index = _mm256_loadu_epi32(indices);
	//__m512d input = _mm512_load_pd(arr);

}


// In case of size 2 memory allocation is 2					= 2 
// In case of size 3 memory allocation is 3 + 2				= 5 
// In case of size 4 memory allocation is 4 + 2				= 6 
// In case of size 5 memory allocation is 5 + 5				= 10
// In case of size 6 memory allocation is 6 + 5				= 11
// In case of size 7 memory allocation is 7 + 6				= 13
// In case of size 8 memory allocation is 8 + 6				= 14
// In case of size 9 memory allocation is 9 + 10			= 19
// In case of size 10 memory allocation is 10 + 10			= 20
// In case of size 11 memory allocation is 11 + 11			= 22
// In case of size 12 memory allocation is 12 + 11			= 23
// In case of size 13 memory allocation is 13 + 13			= 26
// In case of size 14 memory allocation is 14 + 13			= 27
// In case of size 15 memory allocation is 15 + 14			= 29
// In case of size 16 memory allocation is 16 + 14			= 30
// In general case memory allocation is size + size/2


void indexed_merge_sort(double* arr, int* indices, int* temp_workspace, int lo, int hi, int size) {
    if (hi - lo < 2) return; // no need to merge
	if (hi - lo == 2) {
		if (arr[indices[lo]] > arr[indices[lo + 1]]) {
			int temp = indices[lo];
			indices[lo] = indices[lo + 1];
			indices[lo + 1] = temp;
		}
		return;
	}
	int mid = (hi + lo) >> 1;
	indexed_merge_sort(arr, indices, temp_workspace, lo, mid, size);
	indexed_merge_sort(arr, indices, temp_workspace, mid, hi, size);
	// Merge the two halves
	int i = lo;
	int j = mid;
	int k = lo;
	while (i < mid && j < hi) {
		if (arr[indices[i]] <= arr[indices[j]]) {
			temp_workspace[k++] = indices[i++];
		} else {
			temp_workspace[k++] = indices[j++];
		}
	}
	while (i < mid) {
		temp_workspace[k++] = indices[i++];
	}
	while (j < hi) {
		temp_workspace[k++] = indices[j++];
	}
	for (int m = lo; m < hi; m++) {
		indices[m] = temp_workspace[m];
    }
	return;
}


//void indexed_merge_sort(double* arr, int* indices, int* temp_workspace, int size) {
//    merge(arr, indices, temp_workspace, 0, size, size);
//}

static void post_process_population(gene_pool_t* gene_pool, population_param_t* pop_param) {
	int unique = 1;
	// eliminate duplicates
	for (int i = pop_param->reseed_bottom_N; i < gene_pool->individuals - 1; i++) {
		unique = 1;
		if (gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] == gene_pool->pop_result_set[gene_pool->sorted_indexes[i + 1]]) {
			for (int k = 0; k < gene_pool->genes; k++) {
				if (gene_pool->pop_param_bin[gene_pool->sorted_indexes[i]][k] == gene_pool->pop_param_bin[gene_pool->sorted_indexes[i + 1]][k]) {
					unique = 0;
					break;
				}
			}

			if (unique == 0) {
				fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
			}
		}
	}
    // reseed bottom N
    for (int i = 0; i < pop_param->reseed_bottom_N; i++) {
		fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
    }
}

void process_pop(gene_pool_t* gene_pool, task_param_t* task) {
	// TODO: check individual even nr 
	// TODO: refractor individuals and genes to _count

	process_fx(gene_pool, &(task->config_ga.fx_param), task->lower, task->upper); // pop, individuals, genes -> ?

	// worst-best scaling according to fitness and fit function (lin, exp, log, sig, norm)
	process_flatten(gene_pool, &(task->config_ga.flatten_param));

	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->sorted_indexes[i] = i;
	}

	//indexed_merge_sort(gene_pool->flatten_result_set, gene_pool->sorted_indexes, gene_pool->sorted_indexes_temp, 0, gene_pool->individuals, gene_pool->individuals);
	indexed_bitonic_sort(gene_pool->flatten_result_set, gene_pool->sorted_indexes, gene_pool->sorted_indexes_temp, gene_pool->individuals);

	// copy sorted to selected
	for (int i = 0; i < gene_pool->individuals; i++) {
		gene_pool->selected_indexes[i] = gene_pool->sorted_indexes[i];
	}

	process_selection(gene_pool, &(task->config_ga.selection_param));

	// crossover
	process_crossover(gene_pool, &(task->config_ga.crossover_param));

	// mutation
	process_mutation(gene_pool, &(task->config_ga.mutation_param));

    // Eliminate duplicates and reseed bottom N
	post_process_population(gene_pool, &(task->config_ga.population_param));
}