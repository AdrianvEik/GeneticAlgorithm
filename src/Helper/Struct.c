
#include <stdlib.h>

#include "Struct.h"
#include "error_handling.h"
#include "../Utility/process.h"
#include "../Utility/pop.h"
#include "../Utility/crossover.h"
#include "../Utility/mutation.h"
#include "../Utility/selection.h"
#include "../Utility/flatten.h"

#include "../Function/Function.h"


logging_param_t default_logging_param() {
	// Setups default logging parameters
	logging_param_t logging_param;

	logging_param.fully_qualified_basename = "C:/temp/GA\0";
	logging_param.top_n_export = 0;
	logging_param.export_interval = 0;
	logging_param.include_config = 0;
	logging_param.write_csv = 1;
	logging_param.config_int_count = 1;
	logging_param.config_double_count = 2;
	logging_param.queue_size = 128;
    logging_param.write_config = 0;
	return logging_param;
}

runtime_param_t default_runtime_param() {
	// Setups default runtime parameters
	runtime_param_t runtime_param;

	runtime_param.individuals = 128;
	runtime_param.genes = 2;
	runtime_param.elitism = 2;
	runtime_param.task_count = 32;
	runtime_param.thread_count = 4;
	runtime_param.zone_enable = 1;
	runtime_param.logging_param = default_logging_param();

	return runtime_param;
}

config_ga_t default_config(runtime_param_t runtime_param) {
	// Setups default configuration
	flatten_param_t flatten_param;
	flatten_param.flatten_method = flatten_method_none;
	flatten_param.flatten_alpha = 1.0f;
	flatten_param.flatten_beta = 0.0f;

	crossover_param_t crossover_param;
	crossover_param.crossover_method = crossover_method_uniform32;
	crossover_param.crossover_prob = 0.5f;

	mutation_param_t mutation_param;
	mutation_param.mutation_method = 0;
	mutation_param.mutation_prob = 0.5;
	mutation_param.mutation_rate = 6;

	fx_param_t fx_param;
	fx_param.fx_method = fx_method_Styblinski_Tang;
	fx_param.fx_optim_mode = 1;
    fx_param.fx_function = NULL;
    fx_param.fx_data_type = fx_data_type_double;

	population_param_t pop_param;
	pop_param.sampling_type = pop_normal;
	pop_param.sigma = 1;
	pop_param.lower = malloc(sizeof(double) * runtime_param.genes);
	pop_param.upper = malloc(sizeof(double) * runtime_param.genes);

    if (pop_param.lower == NULL || pop_param.upper == NULL) {
        perror("Memory allocation failed: default_config");
        exit(255);
    }

	for (int i = 0; i < runtime_param.genes; i++) {
		pop_param.lower[i] = -5.0f;
		pop_param.upper[i] = 5.0f;
	}
	pop_param.reseed_bottom_N = 2;

	selection_param_t selection_param;
	selection_param.selection_method = selection_method_roulette;
	selection_param.selection_div_param = 0.5f;
	selection_param.selection_prob_param = 0.2f;
	selection_param.selection_temp_param = 10.0f;
	selection_param.selection_tournament_size = 4;
    selection_param.selection_rank_distr = 0; // 0: prob_distr, 1: boltzmann_distr

	optimizer_param_t optimizer_param;
	optimizer_param.convergence_moving_window_size = 10;
	optimizer_param.min_mutations = 1;
	optimizer_param.max_mutations = 100;
	optimizer_param.mutation_factor = 1e3;
	optimizer_param.max_iterations = 10000;
	optimizer_param.convergence_threshold = 1e-8;
	optimizer_param.convergence_window = 1000;

	config_ga_t config_ga;
	config_ga.selection_param = selection_param;
	config_ga.flatten_param = flatten_param;
	config_ga.crossover_param = crossover_param;
	config_ga.mutation_param = mutation_param;
	config_ga.fx_param = fx_param;
	config_ga.population_param = pop_param;
	config_ga.optimizer_param = optimizer_param;

	return config_ga;
}

void verify_input_parameters(config_ga_t config_ga, runtime_param_t runtime_param) {
	if (runtime_param.elitism > runtime_param.individuals) EXIT_WITH_ERROR("Elitism cannot be greater than the number of individuals creation", 250);
	if (runtime_param.individuals < 2) EXIT_WITH_ERROR("The number of individuals must be greater than two", 250);
	if (runtime_param.genes < 1) EXIT_WITH_ERROR("The number of genes must be greater than zero", 250);
}