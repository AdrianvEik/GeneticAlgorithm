
#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


struct gene_pool_s {
	char* gene_pool_memory_ptr;
	uint32_t** pop_param_bin;
	uint32_t** pop_param_bin_cross_buffer;
	double** pop_param_double;
	double* pop_result_set;
	double* flatten_result_set;
	double* selection_temp;
	uint32_t* selected_indexes;
	uint32_t* sorted_indexes;
    uint32_t* sorted_indexes_temp;
	uint32_t* fx_ready;
	uint32_t genes;
	uint32_t individuals;
	uint32_t elitism;
	uint32_t iteration_number;
	uint32_t gene_mem_size; // bits
    uint32_t individual_mem_size; // bytes
};


static const int pop_uniform = 0;
static const int pop_normal = 1;

struct population_param_s {
	uint32_t sampling_type; // DEFAULT =0
	uint32_t sigma; // DEFAULT = 1
	double* lower; // DEFAULT = 5
	double* upper; // DEFAULT = 0
	uint32_t reseed_bottom_N; // DEFAULT = 2
};

static const int selection_method_roulette = 0;
static const int selection_method_rank_tournament = 1;
static const int selection_method_rank = 2;
static const int selection_method_rank_space = 3;
static const int selection_method_boltzmann = 4;

struct selection_param_s {
	int selection_method; // DEFAULT = 0
	double selection_div_param; // DEFAULT = 0.5
	double selection_prob_param; // DEFAULT = 0.5
	double selection_temp_param; // DEFAULT = 10
	uint32_t selection_tournament_size; // DEFAULT = 2
    uint32_t selection_rank_distr; // DEFAULT = 0
};

static const int flatten_method_linear = 0;
static const int flatten_method_exponential = 1;
static const int flatten_method_logarithmic = 2;
static const int flatten_method_normalized = 3;
static const int flatten_method_sigmoid = 4;
static const int flatten_method_none = 5;

struct flatten_param_s {
	int flatten_method; // DEFAULT = 5
	double flatten_alpha; // DEFAULT = 1 (> 0)
	double flatten_beta; // DEFAULT = 0
};

static const int crossover_method_single_point = 0;
static const int crossover_method_two_point = 1;
static const int crossover_method_uniform = 2;
static const int crossover_method_complete = 3;


struct crossover_param_s {
	int crossover_method; // DEFAULT = 0
	double crossover_prob; // DEFAULT = 0.5
};

 //TODO: per gene mutation probability and mutation pressure (rank dependant)
struct mutation_param_s {
	int mutation_method; // DEFAULT = 0
	double mutation_prob; // DEFAULT = 0.5
	double* mutation_rate; // DEFAULT = 6
    double mutation_slope; // DEFAULT = 1
    //double mutation_alpha; // DEFAULT = 1
    //double mutation_beta; // DEFAULT = 0
};

static const int fx_method_pointer = -1;
static const int fx_method_Styblinski_Tang = 0;
static const int fx_method_Wheelers_Ridge = 1;

static const int fx_data_type_double = 1;
static const int fx_data_type_int = 2;

static const int fx_optim_mode_minimize = -1;
static const int fx_optim_mode_maximize = 1;

typedef double (*fx_ptr_generic)(void*, uint32_t);

struct fx_param_s {
	int fx_method; // DEFAULT = 0
	int fx_optim_mode; // DEFAULT = 1
    int fx_data_type; // DEFAULT = 0
	fx_ptr_generic fx_function;
};

struct optimizer_param_s {
	double min_mutations; // DEFAULT = 1
	double max_mutations; // DEFAULT = 10
	double mutation_factor; // DEFAULT = 0.1
	double convergence_threshold; // DEFAULT = 1e-8
	uint32_t convergence_moving_window_size; // DEFAULT = 10
	uint32_t max_iterations; // DEFAULT = 1000
	uint32_t convergence_window; // DEFAULT = 100
};

struct config_ga_s {
	struct population_param_s population_param;
	struct selection_param_s selection_param;
	struct flatten_param_s flatten_param;
	struct crossover_param_s crossover_param;
	struct mutation_param_s mutation_param;
	struct fx_param_s fx_param;
    struct optimizer_param_s optimizer_param;
};

struct logging_param_s {
	char* fully_qualified_basename;
    uint32_t top_n_export; // DEFAULT = 1
    uint32_t export_interval; // DEFAULT = 0 (last only)
    int include_config; // DEFAULT = 0 
    int write_csv; // DEFAULT = 1 JSON dump
    int write_bin; // DEFAULT = 0
    uint32_t config_int_count; // DEFAULT = 1
    uint32_t config_double_count; // DEFAULT = 2
    uint32_t queue_size; // DEFAULT = 128
    int write_config; // DEFAULT = 0
    int console_enabled; // DEFAULT = 1
};

struct runtime_param_s {
	uint32_t genes; // DEFAULT = 2
	uint32_t individuals; // DEFAULT = 32
	uint32_t elitism; // DEFAULT = 2
	uint32_t task_count_solver; // DEFAULT = 32
	uint32_t thread_count_solver; // DEFAULT = 4
	uint32_t task_size_fx; // DEFAULT = 32
    uint32_t thread_count_fx; // DEFAULT = 4
    int zone_enable; // DEFAULT = 1
    uint32_t gene_mem_size; // DEFAULT = 32
    uint32_t random_seed; // DEFAULT = 0
    struct logging_param_s logging_param;
};	

typedef struct gene_pool_s gene_pool_t;
typedef struct selection_param_s selection_param_t;
typedef struct flatten_param_s flatten_param_t;
typedef struct crossover_param_s crossover_param_t;
typedef struct mutation_param_s mutation_param_t;
typedef struct fx_param_s fx_param_t;
typedef struct population_param_s population_param_t;
typedef struct config_ga_s config_ga_t;
typedef struct runtime_param_s runtime_param_t;
typedef struct optimizer_param_s optimizer_param_t;
typedef struct logging_param_s logging_param_t;

runtime_param_t default_runtime_param();
config_ga_t default_config(runtime_param_t runtime_param);
void verify_input_parameters(config_ga_t config_ga, runtime_param_t runtime_param);
#endif // STRUCT_H