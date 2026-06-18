#include "process.h"

static void indexed_bubble_sort(double* arr, uint32_t* indices, uint32_t size) {
	uint32_t swapped = 1;
	uint32_t temp_idx;

	for (uint32_t i = 0; i < size && swapped; i++) {
		swapped = 0;
		for (uint32_t j = 0; j < size - i - 1; j++) {
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

static inline void bitonic_sort_1v(__m512d* input, __m256i* index) { // todo update uint32_t to uuint32_t for index
	//__m256i index = _mm256_loadu_epi32(indices);
    //__m512d input = _mm512_load_pd(arr);
    __m512d work;
	__m256i work_idx;
	__mmask8 cmp_res;


	work = _mm512_permutexvar_pd(_mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1), *input);
    //cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xAA);
    cmp_res = _kor_mask8(
		_mm512_mask_cmp_pd_mask(0x55, work, *input, _CMP_LT_OQ),
		_mm512_mask_cmp_pd_mask(0xAA, work, *input, _CMP_NLE_UQ)
	);

    *input = _mm512_mask_mov_pd(*input, cmp_res, work);
	work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), *index));
    *index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(4, 5, 6, 7, 0, 1, 2, 3), *input);
    //cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xCC);
	cmp_res = _kor_mask8(
		_mm512_mask_cmp_pd_mask(0x33, work, *input, _CMP_LT_OQ),
		_mm512_mask_cmp_pd_mask(0xCC, work, *input, _CMP_NLE_UQ)
	);

    *input = _mm512_mask_mov_pd(*input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(4, 5, 6, 7, 0, 1, 2, 3), *index));
    *index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1), *input);
    //cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xAA);
	cmp_res = _kor_mask8(
		_mm512_mask_cmp_pd_mask(0x55, work, *input, _CMP_LT_OQ),
		_mm512_mask_cmp_pd_mask(0xAA, work, *input, _CMP_NLE_UQ)
	);
	*input = _mm512_mask_mov_pd(*input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), *index));
    *index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7), *input);
    //cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xF0);
	cmp_res = _kor_mask8(
		_mm512_mask_cmp_pd_mask(0x0F, work, *input, _CMP_LT_OQ),
		_mm512_mask_cmp_pd_mask(0xF0, work, *input, _CMP_NLE_UQ)
	);
    *input = _mm512_mask_mov_pd(*input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7), *index));
    *index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2), *input);
    //cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xCC);
	cmp_res = _kor_mask8(
		_mm512_mask_cmp_pd_mask(0x33, work, *input, _CMP_LT_OQ),
		_mm512_mask_cmp_pd_mask(0xCC, work, *input, _CMP_NLE_UQ)
	);
    *input = _mm512_mask_mov_pd(*input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(5, 4, 7, 6, 1, 0, 3, 2), *index));
    *index = work_idx;

    work = _mm512_permutexvar_pd(_mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1), *input);
    //cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xAA);
	cmp_res = _kor_mask8(
		_mm512_mask_cmp_pd_mask(0x55, work, *input, _CMP_LT_OQ),
		_mm512_mask_cmp_pd_mask(0xAA, work, *input, _CMP_NLE_UQ)
	);
    *input = _mm512_mask_mov_pd(*input, cmp_res, work);
    work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), *index));
    *index = work_idx;

    //_mm512_store_pd(arr, *input); // store sorted values back to array??
    //_mm256_storeu_epi32(indices, *index);
    // return set *indexes
}

static inline void bitonic_minmax_2v(__m512d* a, __m512d* b, __m256i* index_a, __m256i* index_b) {

	__m512d swap_b = _mm512_permutexvar_pd(_mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7), *b);
	__m256i swap_index_b = _mm256_permutexvar_epi32(_mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7), *index_b);
	__mmask8 cmp_res = _mm512_cmplt_pd_mask(swap_b, *a);

	*b = _mm512_mask_mov_pd(swap_b, cmp_res, *a);
	__m256i work_index_b = _mm256_mask_mov_epi32(swap_index_b, cmp_res, *index_a);
	*a = _mm512_mask_mov_pd(*a, cmp_res, swap_b);
	*index_a = _mm256_mask_mov_epi32(*index_a, cmp_res, swap_index_b);

    *index_b = work_index_b;
}

static inline void post_sort_1v(__m512d* input, __m256i* index) {
	__m512d work;
	__m256i work_idx;
	__mmask8 cmp_res;

	work = _mm512_permutexvar_pd(_mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4), *input);
	cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xF0);
	*input = _mm512_mask_mov_pd(*input, cmp_res, work);
	work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7), *index));
	*index = work_idx;

	work = _mm512_permutexvar_pd(_mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2), *input);
	cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xCC);
	*input = _mm512_mask_mov_pd(*input, cmp_res, work);
	work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(5, 4, 7, 6, 1, 0, 3, 2), *index));
	*index = work_idx;

	work = _mm512_permutexvar_pd(_mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1), *input);
	cmp_res = _kxor_mask8(_mm512_cmplt_pd_mask(work, *input), 0xAA);
	*input = _mm512_mask_mov_pd(*input, cmp_res, work);
	work_idx = _mm256_mask_mov_epi32(*index, cmp_res, _mm256_permutexvar_epi32(_mm256_set_epi32(6, 7, 4, 5, 2, 3, 0, 1), *index));
	*index = work_idx;
}

static inline void post_sort_2v(__m512d* a, __m256i* index_a, __m512d* b, __m256i* index_b) {	
	//__m512d swap_b = _mm512_permutexvar_pd(_mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7), *b);
	//__m256i swap_index_b = _mm256_permutexvar_epi32(_mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7), *index_b);
	__mmask8 cmp_res = _mm512_cmplt_pd_mask(*b, *a);

	__m512d work_b = _mm512_mask_mov_pd(*b, cmp_res, *a);
	__m256i work_index_b = _mm256_mask_mov_epi32(*index_b, cmp_res, *index_a);
	*a = _mm512_mask_mov_pd(*a, cmp_res, work_b);
	*index_a = _mm256_mask_mov_epi32(*index_a, cmp_res, *index_b);

    *b = work_b;
	*index_b = work_index_b;
}

static inline void bitonic_sort_2v(__m512d* a, __m512d* b, __m256i* index_a, __m256i* index_b) {
	bitonic_sort_1v(a, index_a);
    bitonic_sort_1v(b, index_b);
	bitonic_minmax_2v(a, b, index_a, index_b);
	post_sort_1v(a, index_a);
	post_sort_1v(b, index_b);
}

static inline void bitonic_sort_4v(
	__m512d* a,
	__m512d* b,
	__m512d* c,
	__m512d* d,
	__m256i* index_a,
	__m256i* index_b,
	__m256i* index_c,
	__m256i* index_d
) {
    bitonic_sort_2v(a, b, index_a, index_b);
	bitonic_sort_2v(c, d, index_c, index_d);

	bitonic_minmax_2v(a, d, index_a, index_d);
	bitonic_minmax_2v(b, c, index_b, index_c);
    post_sort_2v(a, index_a, b, index_b);
    post_sort_2v(c, index_c, d, index_d);

    post_sort_1v(a, index_a);
    post_sort_1v(b, index_b);
    post_sort_1v(c, index_c);
    post_sort_1v(d, index_d);
}

static inline void bitonic_sort_8v(
	__m512d* a,
	__m512d* b,
	__m512d* c,
	__m512d* d,
	__m512d* e,
	__m512d* f,
	__m512d* g,
	__m512d* h,
	__m256i* index_a,
	__m256i* index_b,
	__m256i* index_c,
	__m256i* index_d,
	__m256i* index_e,
	__m256i* index_f,
	__m256i* index_g,
	__m256i* index_h
) {
    bitonic_sort_4v(a, b, c, d, index_a, index_b, index_c, index_d);
    bitonic_sort_4v(e, f, g, h, index_e, index_f, index_g, index_h);

	bitonic_minmax_2v(a, h, index_a, index_h);
    bitonic_minmax_2v(b, g, index_b, index_g);
	bitonic_minmax_2v(c, f, index_c, index_f);
    bitonic_minmax_2v(d, e, index_d, index_e);

    post_sort_2v(a, index_a, e, index_e);
    post_sort_2v(b, index_b, f, index_f);
    post_sort_2v(c, index_c, g, index_g);
    post_sort_2v(d, index_d, h, index_h);

	post_sort_2v(a, index_a, b, index_b);
	post_sort_2v(c, index_c, d, index_d);
    post_sort_2v(e, index_e, f, index_f);
    post_sort_2v(g, index_g, h, index_h);

	post_sort_1v(a, index_a);
	post_sort_1v(b, index_b);
	post_sort_1v(c, index_c);
	post_sort_1v(d, index_d);
    post_sort_1v(e, index_e);
    post_sort_1v(f, index_f);
    post_sort_1v(g, index_g);
    post_sort_1v(h, index_h);
}

static void bitonic_sort_16v(
	__m512d* a,
	__m512d* b,
	__m512d* c,
	__m512d* d,
	__m512d* e,
	__m512d* f,
	__m512d* g,
	__m512d* h,
	__m512d* i,
	__m512d* j,
	__m512d* k,
	__m512d* l,
	__m512d* m,
	__m512d* n,
	__m512d* o,
	__m512d* p,
	__m256i* index_a,
	__m256i* index_b,
	__m256i* index_c,
	__m256i* index_d,
	__m256i* index_e,
	__m256i* index_f,
	__m256i* index_g,
	__m256i* index_h,
	__m256i* index_i,
	__m256i* index_j,
	__m256i* index_k,
	__m256i* index_l,
	__m256i* index_m,
	__m256i* index_n,
	__m256i* index_o,
	__m256i* index_p
) {
	bitonic_sort_8v(
		a, b, c, d, e, f, g, h,
		index_a, index_b, index_c, index_d, index_e, index_f, index_g, index_h
    );
    bitonic_sort_8v(
        i, j, k, l, m, n, o, p,
        index_i, index_j, index_k, index_l, index_m, index_n, index_o, index_p
    );

    bitonic_minmax_2v(a, p, index_a, index_p);
    bitonic_minmax_2v(b, o, index_b, index_o);
    bitonic_minmax_2v(c, n, index_c, index_n);
    bitonic_minmax_2v(d, m, index_d, index_m);
    bitonic_minmax_2v(e, l, index_e, index_l);
    bitonic_minmax_2v(f, k, index_f, index_k);
    bitonic_minmax_2v(g, j, index_g, index_j);
    bitonic_minmax_2v(h, i, index_h, index_i);

    post_sort_2v(a, index_a, i, index_i);
    post_sort_2v(b, index_b, j, index_j);
    post_sort_2v(c, index_c, k, index_k);
    post_sort_2v(d, index_d, l, index_l);
    post_sort_2v(e, index_e, m, index_m);
    post_sort_2v(f, index_f, n, index_n);
    post_sort_2v(g, index_g, o, index_o);
    post_sort_2v(h, index_h, p, index_p);

    post_sort_2v(a, index_a, e, index_e);
    post_sort_2v(b, index_b, f, index_f);
    post_sort_2v(c, index_c, g, index_g);
    post_sort_2v(d, index_d, h, index_h);
    post_sort_2v(i, index_i, m, index_m);
    post_sort_2v(j, index_j, n, index_n);
    post_sort_2v(k, index_k, o, index_o);
    post_sort_2v(l, index_l, p, index_p);

    post_sort_2v(a, index_a, c, index_c);
    post_sort_2v(b, index_b, d, index_d);
    post_sort_2v(e, index_e, g, index_g);
    post_sort_2v(f, index_f, h, index_h);
    post_sort_2v(i, index_i, k, index_k);
    post_sort_2v(j, index_j, l, index_l);
    post_sort_2v(m, index_m, o, index_o);
    post_sort_2v(n, index_n, p, index_p);

	post_sort_2v(a, index_a, b, index_b);
	post_sort_2v(c, index_c, d, index_d);
	post_sort_2v(e, index_e, f, index_f);
	post_sort_2v(g, index_g, h, index_h);
	post_sort_2v(i, index_i, j, index_j);
	post_sort_2v(k, index_k, l, index_l);
	post_sort_2v(m, index_m, n, index_n);
    post_sort_2v(o, index_o, p, index_p);

	post_sort_1v(a, index_a);
	post_sort_1v(b, index_b);
	post_sort_1v(c, index_c);
	post_sort_1v(d, index_d);
	post_sort_1v(e, index_e);
	post_sort_1v(f, index_f);
	post_sort_1v(g, index_g);
	post_sort_1v(h, index_h);
	post_sort_1v(i, index_i);
	post_sort_1v(j, index_j);
	post_sort_1v(k, index_k);
	post_sort_1v(l, index_l);
	post_sort_1v(m, index_m);
	post_sort_1v(n, index_n);
	post_sort_1v(o, index_o);
    post_sort_1v(p, index_p);
}

//static void bottom_up_merge(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t lo, uint32_t hi, uint32_t size) {
//    // merge blocks of size 8 to 16 , 16 to 32, 32 to 64, etc
//
//
//}


static void indexed_bitonic_sort_1v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size) {

	for (uint32_t i = 0; i < size / 8; i++) {
		__m256i index_a = _mm256_loadu_epi32(&indices[i * 8]);
		__m512d input_a = _mm512_loadu_pd(&arr[i * 8]);

		bitonic_sort_1v(&input_a, &index_a);
		_mm256_storeu_epi32(&indices[i * 8], index_a);
	}

	// remainder
	uint32_t remaining = size & 0x07;

	if (remaining) {
		uint32_t start = size & 0xFFF8;
		double temp_arr[8] = { DBL_MAX };
		uint32_t temp_idx[8] = { UINT32_MAX };
		for (uint32_t i = 0; i < remaining; i++) {
			temp_arr[i] = arr[start + i];
			temp_idx[i] = indices[start + i];
		}
		__m256i index = _mm256_loadu_epi32(temp_idx);
		__m512d input = _mm512_loadu_pd(temp_arr);
		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(temp_idx, index);
		uint32_t skip_idx = 0;
		for (uint32_t i = 0; i < remaining; i++) {
			if (temp_idx[i] == UINT32_MAX) {
				skip_idx++;
				continue;
			}
			indices[start + i] = temp_idx[i + skip_idx];
		}
	}

	uint32_t i, mid, j, lo, hi, k, iter = 0;

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
			for (uint32_t m = lo; m < hi; m++) {
				indices[m] = temp_workspace[m];
			}
		}
	}
	// TODO handle remaining elements <8
	// TODO handle case when size <8

	//__m256i index = _mm256_loadu_epi32(indices);
	//__m512d input = _mm512_load_pd(arr);

}

static void indexed_bitonic_sort_2v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size ) {

	for (uint32_t i = 0; i < size; i+=16) {
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

        __m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
        __m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

		bitonic_sort_2v(&input_a, &input_b, &index_a, &index_b);
		_mm256_storeu_epi32(&indices[i], index_a);
        _mm256_storeu_epi32(&indices[i + 8], index_b);
	}

	// remainder
	uint32_t remaining = size & 0x0F;

	if (remaining >= 7) {

        uint32_t i = size & 0xFFF0;
		__m256i index = _mm256_loadu_epi32(&indices[i]);
		__m512d input = _mm512_loadu_pd(&arr[i]);

		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(&indices[i], index);

        remaining -= 8;
	}

	if (remaining) {
		uint32_t start = size & 0xFFF8;
		double temp_arr[8] = { DBL_MAX };
		uint32_t temp_idx[8] = { UINT32_MAX };
		for (uint32_t i = 0; i < remaining; i++) {
			temp_arr[i] = arr[start + i];
			temp_idx[i] = indices[start + i];
		}
		__m256i index = _mm256_loadu_epi32(temp_idx);
		__m512d input = _mm512_loadu_pd(temp_arr);
		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(temp_idx, index);
		uint32_t skip_idx = 0;
		for (uint32_t i = 0; i < remaining; i++) {
            if (temp_idx[i] == UINT32_MAX) {
                skip_idx++;
				continue;
			}
			indices[start + i] = temp_idx[i+skip_idx];
		}
    }

	uint32_t i, mid, j, lo, hi, k, iter = 0;

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
			for (uint32_t m = lo; m < hi; m++) {
				indices[m] = temp_workspace[m];
			}
        }
	}
	// TODO handle remaining elements <8
    // TODO handle case when size <8

	//__m256i index = _mm256_loadu_epi32(indices);
	//__m512d input = _mm512_load_pd(arr);

}


static void indexed_bitonic_sort_4v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size) {

	for (uint32_t i = 0; i < size; i+=32) {
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

		__m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
		__m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

		__m256i index_c = _mm256_loadu_epi32(&indices[i + 16]);
		__m512d input_c = _mm512_loadu_pd(&arr[i + 16]);

		__m256i index_d = _mm256_loadu_epi32(&indices[i + 24]);
		__m512d input_d = _mm512_loadu_pd(&arr[i + 24]);

		bitonic_sort_4v(
            &input_a, &input_b, &input_c, &input_d,
			&index_a, &index_b, &index_c, &index_d
		);
		_mm256_storeu_epi32(&indices[i], index_a);
		_mm256_storeu_epi32(&indices[i + 8], index_b);
        _mm256_storeu_epi32(&indices[i + 16], index_c);
        _mm256_storeu_epi32(&indices[i + 24], index_d);
	}

	// remainder
	uint32_t remaining = size & 0x1F;

	if (remaining >= 15) {

		uint32_t i = size & 0xFFE0;
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

        __m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
        __m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

		bitonic_sort_2v(&input_a, &input_b, &index_a, &index_b);
		_mm256_storeu_epi32(&indices[i], index_a);
        _mm256_storeu_epi32(&indices[i + 8], index_b);

		remaining -= 16;
	}

	if (remaining >= 7) {

		uint32_t i = size & 0xFFF0;
		__m256i index = _mm256_loadu_epi32(&indices[i]);
		__m512d input = _mm512_loadu_pd(&arr[i]);

		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(&indices[i], index);

		remaining -= 8;
	}

	if (remaining) {
		uint32_t start = size & 0xFFF8;
		double temp_arr[8] = { DBL_MAX };
		uint32_t temp_idx[8] = { UINT32_MAX };
		for (uint32_t i = 0; i < remaining; i++) {
			temp_arr[i] = arr[start + i];
			temp_idx[i] = indices[start + i];
		}
		__m256i index = _mm256_loadu_epi32(temp_idx);
		__m512d input = _mm512_loadu_pd(temp_arr);
		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(temp_idx, index);
		uint32_t skip_idx = 0;
		for (uint32_t i = 0; i < remaining; i++) {
			if (temp_idx[i] == UINT32_MAX) {
				skip_idx++;
				continue;
			}
			indices[start + i] = temp_idx[i + skip_idx];
		}
	}

	uint32_t i, mid, j, lo, hi, k, iter = 0;

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
			for (uint32_t m = lo; m < hi; m++) {
				indices[m] = temp_workspace[m];
			}
		}
	}

}

static void indexed_bitonic_sort_8v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size) {
	
	for (uint32_t i = 0; i < size; i+=64) {
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

		__m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
		__m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

		__m256i index_c = _mm256_loadu_epi32(&indices[i + 16]);
		__m512d input_c = _mm512_loadu_pd(&arr[i + 16]);

		__m256i index_d = _mm256_loadu_epi32(&indices[i + 24]);
		__m512d input_d = _mm512_loadu_pd(&arr[i + 24]);

        __m256i index_e = _mm256_loadu_epi32(&indices[i + 32]);
        __m512d input_e = _mm512_loadu_pd(&arr[i + 32]);

        __m256i index_f = _mm256_loadu_epi32(&indices[i + 40]);
        __m512d input_f = _mm512_loadu_pd(&arr[i + 40]);

        __m256i index_g = _mm256_loadu_epi32(&indices[i + 48]);
        __m512d input_g = _mm512_loadu_pd(&arr[i + 48]);

        __m256i index_h = _mm256_loadu_epi32(&indices[i + 56]);
        __m512d input_h = _mm512_loadu_pd(&arr[i + 56]);


		bitonic_sort_8v(
			&input_a, &input_b, &input_c, &input_d,
			&input_e, &input_f, &input_g, &input_h,
			&index_a, &index_b, &index_c, &index_d,
			&index_e, &index_f, &index_g, &index_h
        );

		_mm256_storeu_epi32(&indices[i], index_a);
		_mm256_storeu_epi32(&indices[i + 8], index_b);
		_mm256_storeu_epi32(&indices[i + 16], index_c);
		_mm256_storeu_epi32(&indices[i + 24], index_d);
		_mm256_storeu_epi32(&indices[i + 32], index_e);
		_mm256_storeu_epi32(&indices[i + 40], index_f);
		_mm256_storeu_epi32(&indices[i + 48], index_g);
        _mm256_storeu_epi32(&indices[i + 56], index_h);
	}

	// remainder
	uint32_t remaining = size & 0x3F;

	if (remaining >= 31) {

		uint32_t i = size & 0xFFC0;
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

		__m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
		__m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

        __m256i index_c = _mm256_loadu_epi32(&indices[i + 16]);
        __m512d input_c = _mm512_loadu_pd(&arr[i + 16]);

        __m256i index_d = _mm256_loadu_epi32(&indices[i + 24]);
        __m512d input_d = _mm512_loadu_pd(&arr[i + 24]);

		bitonic_sort_4v(
			&input_a, &input_b, &input_c, &input_d,
			&index_a, &index_b, &index_c, &index_d
        );

		_mm256_storeu_epi32(&indices[i], index_a);
		_mm256_storeu_epi32(&indices[i + 8], index_b);
		_mm256_storeu_epi32(&indices[i + 16], index_c);
        _mm256_storeu_epi32(&indices[i + 24], index_d);

		remaining -= 32;
	}


	if (remaining >= 15) {

		uint32_t i = size & 0xFFE0;
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

		__m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
		__m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

		bitonic_sort_2v(&input_a, &input_b, &index_a, &index_b);
		_mm256_storeu_epi32(&indices[i], index_a);
		_mm256_storeu_epi32(&indices[i + 8], index_b);

		remaining -= 16;
	}

	if (remaining >= 7) {

		uint32_t i = size & 0xFFF0;
		__m256i index = _mm256_loadu_epi32(&indices[i]);
		__m512d input = _mm512_loadu_pd(&arr[i]);

		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(&indices[i], index);

		remaining -= 8;
	}

	if (remaining) {
		uint32_t start = size & 0xFFF8;
		double temp_arr[8] = { DBL_MAX };
		uint32_t temp_idx[8] = { UINT32_MAX };
		for (uint32_t i = 0; i < remaining; i++) {
			temp_arr[i] = arr[start + i];
			temp_idx[i] = indices[start + i];
		}
		__m256i index = _mm256_loadu_epi32(temp_idx);
		__m512d input = _mm512_loadu_pd(temp_arr);
		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(temp_idx, index);
		uint32_t skip_idx = 0;
		for (uint32_t i = 0; i < remaining; i++) {
			if (temp_idx[i] == UINT32_MAX) {
				skip_idx++;
				continue;
			}
			indices[start + i] = temp_idx[i + skip_idx];
		}
	}

	uint32_t i, mid, j, lo, hi, k, iter = 0;

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
			for (uint32_t m = lo; m < hi; m++) {
				indices[m] = temp_workspace[m];
			}
		}
	}

}

static void indexed_bitonic_sort_16v(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t size) {

	for (uint32_t i = 0; i < size; i += 128) {
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

		__m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
		__m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

		__m256i index_c = _mm256_loadu_epi32(&indices[i + 16]);
		__m512d input_c = _mm512_loadu_pd(&arr[i + 16]);

		__m256i index_d = _mm256_loadu_epi32(&indices[i + 24]);
		__m512d input_d = _mm512_loadu_pd(&arr[i + 24]);

		__m256i index_e = _mm256_loadu_epi32(&indices[i + 32]);
		__m512d input_e = _mm512_loadu_pd(&arr[i + 32]);

		__m256i index_f = _mm256_loadu_epi32(&indices[i + 40]);
		__m512d input_f = _mm512_loadu_pd(&arr[i + 40]);

		__m256i index_g = _mm256_loadu_epi32(&indices[i + 48]);
		__m512d input_g = _mm512_loadu_pd(&arr[i + 48]);

		__m256i index_h = _mm256_loadu_epi32(&indices[i + 56]);
		__m512d input_h = _mm512_loadu_pd(&arr[i + 56]);

        __m256i index_i = _mm256_loadu_epi32(&indices[i + 64]);
        __m512d input_i = _mm512_loadu_pd(&arr[i + 64]);

        __m256i index_j = _mm256_loadu_epi32(&indices[i + 72]);
        __m512d input_j = _mm512_loadu_pd(&arr[i + 72]);

        __m256i index_k = _mm256_loadu_epi32(&indices[i + 80]);
        __m512d input_k = _mm512_loadu_pd(&arr[i + 80]);

        __m256i index_l = _mm256_loadu_epi32(&indices[i + 88]);
        __m512d input_l = _mm512_loadu_pd(&arr[i + 88]);

        __m256i index_m = _mm256_loadu_epi32(&indices[i + 96]);
        __m512d input_m = _mm512_loadu_pd(&arr[i + 96]);

        __m256i index_n = _mm256_loadu_epi32(&indices[i + 104]);
        __m512d input_n = _mm512_loadu_pd(&arr[i + 104]);

        __m256i index_o = _mm256_loadu_epi32(&indices[i + 112]);
        __m512d input_o = _mm512_loadu_pd(&arr[i + 112]);

        __m256i index_p = _mm256_loadu_epi32(&indices[i + 120]);
        __m512d input_p = _mm512_loadu_pd(&arr[i + 120]);


        bitonic_sort_16v(
            &input_a, &input_b, &input_c, &input_d,
            &input_e, &input_f, &input_g, &input_h,
            &input_i, &input_j, &input_k, &input_l,
            &input_m, &input_n, &input_o, &input_p,
            &index_a, &index_b, &index_c, &index_d,
            &index_e, &index_f, &index_g, &index_h,
            &index_i, &index_j, &index_k, &index_l,
            &index_m, &index_n, &index_o, &index_p
        );

		_mm256_storeu_epi32(&indices[i], index_a);
		_mm256_storeu_epi32(&indices[i + 8], index_b);
		_mm256_storeu_epi32(&indices[i + 16], index_c);
		_mm256_storeu_epi32(&indices[i + 24], index_d);
		_mm256_storeu_epi32(&indices[i + 32], index_e);
		_mm256_storeu_epi32(&indices[i + 40], index_f);
		_mm256_storeu_epi32(&indices[i + 48], index_g);
		_mm256_storeu_epi32(&indices[i + 56], index_h);
        _mm256_storeu_epi32(&indices[i + 64], index_i);
        _mm256_storeu_epi32(&indices[i + 72], index_j);
        _mm256_storeu_epi32(&indices[i + 80], index_k);
        _mm256_storeu_epi32(&indices[i + 88], index_l);
        _mm256_storeu_epi32(&indices[i + 96], index_m);
        _mm256_storeu_epi32(&indices[i + 104], index_n);
        _mm256_storeu_epi32(&indices[i + 112], index_o);
        _mm256_storeu_epi32(&indices[i + 120], index_p);
	}

	// remainder
	uint32_t remaining = size & 0x7F;

	if (remaining >= 63) {
		uint32_t i = size & 0xFFC0;
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);
		__m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
		__m512d input_b = _mm512_loadu_pd(&arr[i + 8]);
		__m256i index_c = _mm256_loadu_epi32(&indices[i + 16]);
		__m512d input_c = _mm512_loadu_pd(&arr[i + 16]);
		__m256i index_d = _mm256_loadu_epi32(&indices[i + 24]);
		__m512d input_d = _mm512_loadu_pd(&arr[i + 24]);
		__m256i index_e = _mm256_loadu_epi32(&indices[i + 32]);
		__m512d input_e = _mm512_loadu_pd(&arr[i + 32]);
		__m256i index_f = _mm256_loadu_epi32(&indices[i + 40]);
		__m512d input_f = _mm512_loadu_pd(&arr[i + 40]);
		__m256i index_g = _mm256_loadu_epi32(&indices[i + 48]);
		__m512d input_g = _mm512_loadu_pd(&arr[i + 48]);
		__m256i index_h = _mm256_loadu_epi32(&indices[i + 56]);
		__m512d input_h = _mm512_loadu_pd(&arr[i + 56]);
		bitonic_sort_8v(
			&input_a, &input_b, &input_c, &input_d,
			&input_e, &input_f, &input_g, &input_h,
			&index_a, &index_b, &index_c, &index_d,
			&index_e, &index_f, &index_g, &index_h
		);
		_mm256_storeu_epi32(&indices[i], index_a);
		_mm256_storeu_epi32(&indices[i + 8], index_b);
        _mm256_storeu_epi32(&indices[i + 16], index_c);
        _mm256_storeu_epi32(&indices[i + 24], index_d);
        _mm256_storeu_epi32(&indices[i + 32], index_e);
        _mm256_storeu_epi32(&indices[i + 40], index_f);
        _mm256_storeu_epi32(&indices[i + 48], index_g);
        _mm256_storeu_epi32(&indices[i + 56], index_h);
        remaining -= 64;
    }

	if (remaining >= 31) {

		uint32_t i = size & 0xFFC0;
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

		__m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
		__m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

		__m256i index_c = _mm256_loadu_epi32(&indices[i + 16]);
		__m512d input_c = _mm512_loadu_pd(&arr[i + 16]);

		__m256i index_d = _mm256_loadu_epi32(&indices[i + 24]);
		__m512d input_d = _mm512_loadu_pd(&arr[i + 24]);

		bitonic_sort_4v(
			&input_a, &input_b, &input_c, &input_d,
			&index_a, &index_b, &index_c, &index_d
		);

		_mm256_storeu_epi32(&indices[i], index_a);
		_mm256_storeu_epi32(&indices[i + 8], index_b);
		_mm256_storeu_epi32(&indices[i + 16], index_c);
		_mm256_storeu_epi32(&indices[i + 24], index_d);

		remaining -= 32;
	}


	if (remaining >= 15) {

		uint32_t i = size & 0xFFE0;
		__m256i index_a = _mm256_loadu_epi32(&indices[i]);
		__m512d input_a = _mm512_loadu_pd(&arr[i]);

		__m256i index_b = _mm256_loadu_epi32(&indices[i + 8]);
		__m512d input_b = _mm512_loadu_pd(&arr[i + 8]);

		bitonic_sort_2v(&input_a, &input_b, &index_a, &index_b);
		_mm256_storeu_epi32(&indices[i], index_a);
		_mm256_storeu_epi32(&indices[i + 8], index_b);

		remaining -= 16;
	}

	if (remaining >= 7) {

		uint32_t i = size & 0xFFF0;
		__m256i index = _mm256_loadu_epi32(&indices[i]);
		__m512d input = _mm512_loadu_pd(&arr[i]);

		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(&indices[i], index);

		remaining -= 8;
	}

	if (remaining) {
		uint32_t start = size & 0xFFF8;
		double temp_arr[8] = { DBL_MAX };
		uint32_t temp_idx[8] = { UINT32_MAX };
		for (uint32_t i = 0; i < remaining; i++) {
			temp_arr[i] = arr[start + i];
			temp_idx[i] = indices[start + i];
		}
		__m256i index = _mm256_loadu_epi32(temp_idx);
		__m512d input = _mm512_loadu_pd(temp_arr);
		bitonic_sort_1v(&input, &index);
		_mm256_storeu_epi32(temp_idx, index);
		uint32_t skip_idx = 0;
		for (uint32_t i = 0; i < remaining; i++) {
			if (temp_idx[i] == UINT32_MAX) {
				skip_idx++;
				continue;
			}
			indices[start + i] = temp_idx[i + skip_idx];
		}
	}

	uint32_t i, mid, j, lo, hi, k, iter = 0;

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
			for (uint32_t m = lo; m < hi; m++) {
				indices[m] = temp_workspace[m];
			}
		}
	}

}

static void indexed_merge_sort(double* arr, uint32_t* indices, uint32_t* temp_workspace, uint32_t lo, uint32_t hi, uint32_t size) {
    if (hi - lo < 2) return; // no need to merge
	if (hi - lo == 2) {
		if (arr[indices[lo]] > arr[indices[lo + 1]]) {
			uint32_t temp = indices[lo];
			indices[lo] = indices[lo + 1];
			indices[lo + 1] = temp;
		}
		return;
	}
	uint32_t mid = (hi + lo) >> 1;
	indexed_merge_sort(arr, indices, temp_workspace, lo, mid, size);
	indexed_merge_sort(arr, indices, temp_workspace, mid, hi, size);
	// Merge the two halves
	uint32_t i = lo;
	uint32_t j = mid;
	uint32_t k = lo;
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
	for (uint32_t m = lo; m < hi; m++) {
		indices[m] = temp_workspace[m];
    }
	return;
}


//void indexed_merge_sort(double* arr, int* indices, int* temp_workspace, int size) {
//    merge(arr, indices, temp_workspace, 0, size, size);
//}

static uint32_t dedupe_population(gene_pool_t* gene_pool) {
	int unique = 1;
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__m512i);
	uint32_t duplicate_count = 0;

	__mAVXi** pop_param_bin_ptr = (__mAVXi**)gene_pool->pop_param_bin;

	// eliminate duplicates
	for (uint32_t i = 0; i < gene_pool->individuals - 1; i++) {
		unique = 0;
		if (gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] == gene_pool->pop_result_set[gene_pool->sorted_indexes[i + 1]]) {
			for (int j = 0; j < memory_blocks; j++) {
				uint8_t mask = _mm512_cmpeq_epu64_mask(pop_param_bin_ptr[gene_pool->sorted_indexes[i]][j], pop_param_bin_ptr[gene_pool->sorted_indexes[i + 1]][j]);
				if (mask != 0xFFFFFFFFFFFFFFFF) {
					unique = 1;
					break;
				}
			}

			if (unique == 0) {
                gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] = -DBL_MAX; // mark duplicate fitness for reseeding
                duplicate_count++;
			}
		}
	}
	return duplicate_count;
}

static void reseed_population(gene_pool_t* gene_pool, population_param_t* pop_param, uint32_t duplicate_count) {
    duplicate_count = (duplicate_count > pop_param->reseed_bottom_N) ? duplicate_count : pop_param->reseed_bottom_N;
	for (uint32_t i = 0; i < duplicate_count; i++) {
		fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
	}
}

static void post_process_population(gene_pool_t* gene_pool, population_param_t* pop_param) {
	int unique = 1;
    int reseed_remaining = pop_param->reseed_bottom_N;
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__m512i);

	__mAVXi** pop_param_bin_ptr = (__mAVXi**)gene_pool->pop_param_bin;

	// eliminate duplicates
	for (uint32_t i = 0; i < gene_pool->individuals - 1; i++) {
		unique = 0;
		if (gene_pool->pop_result_set[gene_pool->sorted_indexes[i]] == gene_pool->pop_result_set[gene_pool->sorted_indexes[i + 1]]) {
			for (int j = 0; j < memory_blocks; j++) {
				uint8_t mask = _mm512_cmpeq_epu64_mask(pop_param_bin_ptr[gene_pool->sorted_indexes[i]][j], pop_param_bin_ptr[gene_pool->sorted_indexes[i + 1]][j]);
				if (mask != 0xFFFFFFFFFFFFFFFF) {
					unique = 1;
					break;
				}
			}

			if (unique == 0) {
				fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
                reseed_remaining--;
			}
		}
	}
    // reseed bottom N
    for (uint32_t i = 0; i < reseed_remaining; i++) {
		fill_individual_uniform(gene_pool, gene_pool->sorted_indexes[i]);
    }
}

void process_pop(gene_pool_t* gene_pool, task_param_t* task, fx_task_queue_t* fx_task_queue) {
	// TODO: check individual even nr 
	// TODO: refractor individuals and genes to _count
	uint32_t dupe_count = 0;

	process_fx(gene_pool, task, fx_task_queue); // pop, individuals, genes -> ?

	//indexed_merge_sort(gene_pool->flatten_result_set, gene_pool->sorted_indexes, gene_pool->sorted_indexes_temp, 0, gene_pool->individuals, gene_pool->individuals);
	for (uint32_t i = 0; i < gene_pool->individuals; i++) {
		gene_pool->sorted_indexes[i] = i;
	}
	indexed_bitonic_sort_1v(gene_pool->flatten_result_set, gene_pool->sorted_indexes, gene_pool->sorted_indexes_temp, gene_pool->individuals);

	dupe_count = dedupe_population(gene_pool);

    indexed_bitonic_sort_1v(gene_pool->flatten_result_set, gene_pool->sorted_indexes, gene_pool->sorted_indexes_temp, gene_pool->individuals);

	// worst-best scaling according to fitness and fit function (lin, exp, log, sig, norm)
	process_flatten(gene_pool, &(task->config_ga.flatten_param));

	// copy sorted to selected
	for (uint32_t i = 0; i < gene_pool->individuals; i++) {
		gene_pool->selected_indexes[i] = gene_pool->sorted_indexes[i];
	}

	process_selection(gene_pool, &(task->config_ga.selection_param));

	// crossover
	process_crossover(gene_pool, &(task->config_ga.crossover_param));

	// mutation
	process_mutation(gene_pool, &(task->config_ga.mutation_param));

    // Eliminate duplicates and reseed bottom N
    reseed_population(gene_pool, &(task->config_ga.population_param), dupe_count);
	//post_process_population(gene_pool, &(task->config_ga.population_param));
}