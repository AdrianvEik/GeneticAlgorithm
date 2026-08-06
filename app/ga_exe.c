
#include <Windows.h>
#include <locale.h>

#include "../src/Genetic_Algorithm.h"

int main() {
	if (setlocale(LC_ALL, "") == NULL) EXIT_WITH_ERROR("Setting locale", 1);


	uint32_t repeats = 1;
	runtime_param_t runtime_param = default_runtime_param();
	runtime_param.zone_enable = 0;
	runtime_param.task_count_solver = 32;
	runtime_param.individuals = 256;
	runtime_param.genes = 32;
	runtime_param.thread_count_solver = 6;
	runtime_param.elitism = 3;
	runtime_param.random_seed = 0u; // random seed

	runtime_param.logging_param.include_config = 1;
	//runtime_param.logging_param.write_config = 1; // JSON dump
	runtime_param.logging_param.write_csv = 1;
	runtime_param.logging_param.write_bin = 0;
	runtime_param.logging_param.export_interval = 0;
	runtime_param.logging_param.top_n_export = 0;
	runtime_param.logging_param.console_enabled = 1;
	runtime_param.task_size_fx = 0;
	runtime_param.thread_count_fx = 1;


	config_ga_t config_ga = default_config(runtime_param);
	config_ga.selection_param.selection_method = selection_method_roulette;
	config_ga.population_param.reseed_bottom_N = 1;
	config_ga.crossover_param.crossover_method = crossover_method_two_point;

	config_ga.optimizer_param.convergence_window = 4000;
	config_ga.optimizer_param.convergence_moving_window_size = 100;
	config_ga.optimizer_param.max_iterations = 10000;
	config_ga.optimizer_param.max_mutations = 300;
	config_ga.optimizer_param.min_mutations = 1;
    config_ga.optimizer_param.convergence_threshold = 1e-20;
	config_ga.mutation_param.mutation_slope = 1.0;

	config_ga.fx_param.fx_method = fx_method_Styblinski_Tang;
	//   config_ga.fx_param.fx_function = &optimize_fx_ga;
	config_ga.fx_param.fx_data_type = fx_data_type_double;
	//config_ga.fx_param.fx_optim_mode = fx_optim_mode_maximize;

	//config_ga.mutation_param.mutation_alpha = 10;
	//config_ga.mutation_param.mutation_beta = 0.1;

	for (uint32_t i = 0; i < repeats; i++) {
		Genetic_Algorithm(config_ga, runtime_param, NULL);
	}
	free_config_ga(&config_ga);

	return 0;
}