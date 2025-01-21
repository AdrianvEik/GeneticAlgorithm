
#ifndef STRUCT_H
#define STRUCT_H

struct gene_pool_s {
	char* gene_pool_memory_ptr;
	unsigned int** pop_param_bin;
	unsigned int** pop_param_bin_cross_buffer;
	double** pop_param_double;
	double* pop_result_set;
	double* flatten_result_set;
	int* selected_indexes;
	int* sorted_indexes;
	int genes;
	int individuals;
	int elitism;
	int iteration_number;
};

struct population_param_s {
	int sampling_type; // DEFAULT =0
	int sigma; // DEFAULT = 1
	double* lower; // DEFAULT = 5
	double* upper; // DEFAULT = 0
    int reseed_bottom_N; // DEFAULT = 2
};

struct selection_param_s {
	int selection_method; // DEFAULT = 0
	double selection_div_param; // DEFAULT = 0.5
	double selection_prob_param; // DEFAULT = 0.5
	double selection_temp_param; // DEFAULT = 10
	int selection_tournament_size; // DEFAULT = 2
    int selection_rank_distr; // DEFAULT = 0
};

struct flatten_param_s {
	int flatten_method; // DEFAULT = 5
	double flatten_alpha; // DEFAULT = 1 (> 0)
	double flatten_beta; // DEFAULT = 0
};

struct crossover_param_s {
	int crossover_method; // DEFAULT = 0
	double crossover_prob; // DEFAULT = 0.5
};

 //TODO: per gene mutation probability and mutation pressure (rank dependant)
struct mutation_param_s {
	int mutation_method; // DEFAULT = 0
	double mutation_prob; // DEFAULT = 0.5
	int mutation_rate; // DEFAULT = 6
    double mutation_alpha; // DEFAULT = 1
    double mutation_beta; // DEFAULT = 0
};

typedef double (*fx_ptr_generic)(void*, int);

struct fx_param_s {
	int fx_method; // DEFAULT = 0
	int fx_optim_mode; // DEFAULT = 0
    int fx_data_type; // DEFAULT = 0
	fx_ptr_generic fx_function;
};

struct optimizer_param_s {
	int convergence_moving_window_size; // DEFAULT = 10
	int min_mutations; // DEFAULT = 1
	int max_mutations; // DEFAULT = 10
	double mutation_factor; // DEFAULT = 0.1
	int max_iterations; // DEFAULT = 1000
	double convergence_threshold; // DEFAULT = 1e-8
	int convergence_window; // DEFAULT = 100
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
    int top_n_export; // DEFAULT = 1
    int export_interval; // DEFAULT = 0 (last only)
    int include_config; // DEFAULT = 0
    int write_csv; // DEFAULT = 1
    int config_int_count; // DEFAULT = 1
    int config_double_count; // DEFAULT = 2
    int queue_size; // DEFAULT = 128
    int write_config; // DEFAULT = 0
};

struct runtime_param_s {
	int genes; // DEFAULT = 2
	int individuals; // DEFAULT = 32
	int elitism; // DEFAULT = 2
	int task_count; // DEFAULT = 32
	int thread_count; // DEFAULT = 4
    int zone_enable; // DEFAULT = 1
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