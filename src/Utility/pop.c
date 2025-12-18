
#include "pop.h"

#define PI   3.14159265358979323846264338327950288419716939937510f

static void populate_uniform(uint32_t genes, uint32_t* result) {

	/*
	Fill a vector with uniformly distributed random bits.

	:param genes: The number of genes in an individual.
	:type genes: int


	:param result: The matrix to be filled with random bits.
				   shape = (genes )
	:type result: int*

	*/

	for (uint32_t j = 0; j < genes; j++) {
		result[j] = gen_mt_rand();
	}

}

static inline uint32_t double2int(double val, double lower, double upper) {
    /*
    Convert an integer to a double. andersom!

    :param val: The integer to be converted to a double.
    :type val: int

    :return: The double.
    :rtype: int
    */	
	return (uint32_t)(((val - lower) / (upper - lower)) * UINT32_MAX);
}

static void populate_normal_box_muller(uint32_t** result, uint32_t individuals, uint32_t genes) {
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
	for (uint32_t i = 0; i < individuals; i++) {
		for (uint32_t j = 0; j < genes; j += 2) { 
            U1 = ((double) gen_mt_rand() / UINT32_MAX);
            U2 = ((double) gen_mt_rand() / UINT32_MAX);

			// ln 0 = inf
            if (U1 == 0) {
                U1 = 1;
            }

			z1 = sqrt(-2 * log(U1)) * cos(2 * PI * U2);
			z2 = sqrt(-2 * log(U1)) * sin(2 * PI * U2); 

            // Box muller generates normalised values between -6.7 and 6.7 (using int32 resolution)
			result[i][j] = double2int((z1), -6.7, 6.7);
            if (j <= genes) { // Check if the next gene is within the bounds of the genes
				result[i][j+1] = double2int((z2), -6.7, 6.7);
			}
		}
        
	}
}

//inline double cauchy(double x, double mu, double sigma) {
//	/*
//	Calculate the cauchy of x
//
//	x is the input
//	mu is the mean
//	sigma is the standard deviation
//	*/
//
//	return (1 / PI) * (sigma / (pow(x - mu, 2) + pow(sigma, 2)));
//}
//
//static void populate_cauchy(int** result, int individuals, int genes, population_param_t pop_param) {
//	/*
//
//	Produce a normal distributed set of values using the Cauchy distribution:
//
//	.. math::
//		f(x) = \frac{1}{\pi \gamma [1 + (\frac{x - x_0}{\gamma})^2]}
//
//	Where x is linearly spaced between (-factor and factor) + bias.
//
//	:param bitsize: The size of the bitstring.
//	:type bitsize: int
//
//	:param genes: The number of genes in the bitstring.
//	:type genes: int
//
//	:param individuals: The number of individuals in the bitstring.
//	:type individuals: int
//
//	:param result: The matrix to be filled with bits according to a normal distribution.
//				   shape = (individuals, genes * bitsize)
//	:type result: int**
//
//	*/
//	// Determine the steps between the values in the normal distribution
//		// make scale and loc in for loop
//	double scale;
//	double loc;
//
//	double cauchydouble;
//	double scaledcauchy;
//
//	for (uint32_t i = 0; i < individuals; i++) {
//		for (uint32_t j = 0; j < genes; j++) {
//			scale = 1 / pop_param.sigma * (pop_param.upper[i] - pop_param.lower[i]) / 2;
//			loc = (pop_param.upper[i] + pop_param.lower[i]) / 2;
//			
//			cauchydouble = cauchy(gen_mt_rand64(), 0, 1); //TODO: casting int to double produces undesirable results
//            scaledcauchy = (cauchydouble * scale) + loc;
//			
//			result[i][j] = double2int(scaledcauchy, pop_param.lower[i], pop_param.upper[i]);
//		}
//	}
//}

void init_gene_pool(gene_pool_t* gene_pool, runtime_param_t* runtime_param) {
	//gene_pool_t {
	// int** pop_param_bin;
	// double** pop_param_double;
	// double* pop_result_set;
	// int* selected_indexes;
	// int genes;
	// int individuals;
	// int elitism;

	gene_pool->genes = runtime_param->genes;
	gene_pool->individuals = runtime_param->individuals;
	gene_pool->elitism = runtime_param->elitism;
	gene_pool->gene_mem_size = runtime_param->gene_mem_size;

	uint64_t total_memsize = 0;
	uintptr_t current_mem_ptr = 0;

    // Calculate the total memory size needed
    total_memsize += gene_pool->individuals * sizeof(double); // flatten result set
    total_memsize += gene_pool->individuals * sizeof(uint32_t*); // pop_param_bin
    total_memsize += gene_pool->individuals * sizeof(uint32_t*); // pop_param_bin_cross_buffer
	total_memsize += gene_pool->individuals * sizeof(double*); // pop_param_double
    total_memsize += gene_pool->individuals * sizeof(double); // pop_result_set
    total_memsize += gene_pool->individuals * sizeof(double); // selection_temp
    total_memsize += gene_pool->individuals * sizeof(uint32_t); // selected_indexes
    total_memsize += gene_pool->individuals * sizeof(uint32_t); // sorted_indexes
	total_memsize += gene_pool->individuals * sizeof(uint32_t); // sorted_indexes_temp
	total_memsize += gene_pool->individuals * sizeof(uint32_t); // fx_ready


#if defined __AVX512VL__
    gene_pool->individual_mem_size = ((uint64_t)(gene_pool->genes * gene_pool->gene_mem_size + 511) / 512) * sizeof(__m512i);
#else
	gene_pool->individual_mem_size = ((gene_pool->genes * gene_pool->gene_mem_size + 255) / 256) * sizeof(__m256i);
#endif

	total_memsize += (uint64_t)gene_pool->individuals * gene_pool->individual_mem_size; // pop_param_bin
	total_memsize += (uint64_t)gene_pool->individuals * gene_pool->individual_mem_size; // pop_param_bin_cross_buffer
	total_memsize += (uint64_t)gene_pool->individuals * gene_pool->genes * sizeof(double);

    // Allocate the memory
	if ((gene_pool->gene_pool_memory_ptr = _aligned_malloc(total_memsize, AVX_bits)) == NULL) EXIT_MEM_ERROR();
	 

	// pointers to blocks
	current_mem_ptr = (uintptr_t) gene_pool->gene_pool_memory_ptr + (uintptr_t) 2 * gene_pool->individuals * gene_pool->individual_mem_size;
    gene_pool->flatten_result_set = (double*) current_mem_ptr;
	current_mem_ptr += gene_pool->individuals * sizeof(double);

    gene_pool->pop_param_bin = (uint32_t**)current_mem_ptr ;
    current_mem_ptr += gene_pool->individuals * sizeof(int*);

    gene_pool->pop_param_bin_cross_buffer = (uint32_t**)current_mem_ptr;
    current_mem_ptr += gene_pool->individuals * sizeof(uint32_t*);

    gene_pool->pop_param_double = (double**)current_mem_ptr;
    current_mem_ptr += gene_pool->individuals * sizeof(double*);

    gene_pool->pop_result_set = (double*)current_mem_ptr;
    current_mem_ptr += gene_pool->individuals * sizeof(double);

    gene_pool->selection_temp = (double*)current_mem_ptr;
    current_mem_ptr += gene_pool->individuals * sizeof(double);

    gene_pool->selected_indexes = (uint32_t*)current_mem_ptr;
    current_mem_ptr += gene_pool->individuals * sizeof(uint32_t);

    gene_pool->sorted_indexes = (uint32_t*)current_mem_ptr;
    current_mem_ptr += gene_pool->individuals * sizeof(uint32_t);

    gene_pool->sorted_indexes_temp = (uint32_t*)current_mem_ptr;
    current_mem_ptr += gene_pool->individuals * sizeof(uint32_t);

	gene_pool->fx_ready = (uint32_t*)current_mem_ptr;
	current_mem_ptr += gene_pool->individuals * sizeof(uint32_t);

	uint64_t alligned_mem_ptr = (uint64_t)gene_pool->gene_pool_memory_ptr;
    // pointers to data
    for (uint32_t i = 0; i < gene_pool->individuals; i++) {
        gene_pool->pop_param_bin[i] = (uint32_t*)alligned_mem_ptr;
		alligned_mem_ptr += gene_pool->individual_mem_size;

        gene_pool->pop_param_bin_cross_buffer[i] = (uint32_t*)alligned_mem_ptr;
		alligned_mem_ptr += gene_pool->individual_mem_size;

		gene_pool->pop_param_double[i] = (double*)current_mem_ptr;
		current_mem_ptr += gene_pool->genes * sizeof(double);
    }

	if ((uint64_t)gene_pool->gene_pool_memory_ptr + total_memsize != current_mem_ptr) {
		EXIT_WITH_ERROR("Expected memory allocated does not match up with memory allocated", 255);
	}
}
void free_gene_pool(gene_pool_t* gene_pool) {
	_aligned_free(gene_pool->gene_pool_memory_ptr);
}

inline void fill_individual_uniform(gene_pool_t* gene_pool, uint32_t individual) {
	//populate_uniform(gene_pool->genes, gene_pool->pop_param_bin[individual]);
#ifdef __AVX512VL__
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__m512i);
	__m512i* ptr = (__m512i*)gene_pool->pop_param_bin[individual];
	for (uint32_t i = 0; i < memory_blocks; i++) {
		ptr[i] = gen_mt_rand512();
	}
#else 
#ifdef __AVX2__
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(__m256i);
	__m256i* ptr = (__m256i*)gene_pool->pop_param_bin[individual];
	for (uint32_t i = 0; i < memory_blocks; i++) {
		ptr[i] = gen_mt_rand256();
	}
#else
	uint32_t memory_blocks = gene_pool->individual_mem_size / sizeof(uint32_t);
	uint32_t* ptr = (uint32_t*)gene_pool->pop_param_bin[individual];
	for (uint32_t i = 0; i < memory_blocks; i++) {
		ptr[i] = gen_mt_rand();
	}
#endif
#endif
}

void fill_pop(gene_pool_t* gene_pool, population_param_t pop_param, fx_param_t fx_param) {
	if (pop_param.sampling_type == pop_uniform)
		for (uint32_t i = 0; i < gene_pool->individuals; i++) {
			fill_individual_uniform(gene_pool, i);
		}
	else if (pop_param.sampling_type == pop_normal) {
		populate_normal_box_muller(gene_pool->pop_param_bin, gene_pool->individuals, gene_pool->genes);
	}

	for (uint32_t i = 0; i < gene_pool->individuals; i++) {
        gene_pool->pop_result_set[i] =  fx_param.fx_optim_mode == fx_optim_mode_minimize ? DBL_MAX : -DBL_MAX;
        gene_pool->sorted_indexes[i] = i;
	}
	//else if (pop_param.sampling_type == pop_cauchy) {
	//	populate_cauchy(gene_pool->pop_param_bin, gene_pool->individuals, gene_pool->genes, pop_param);
	//}
}