
#include "main.h"

#include "../src/Genetic_Algorithm.h"
#include "../src/Utility/selection.h"
#include "../src/Utility/pop.h"
#include "../src/Utility/mutation.h"
#include "../src/Utility/flatten.h"
#include "../src/Utility/crossover.h"


int main() {
	int repeats = 1;
	runtime_param_t runtime_param = default_runtime_param();
	runtime_param.zone_enable = 0;
	runtime_param.task_count = 64;
	runtime_param.individuals = 128;
	runtime_param.genes = 32;
	runtime_param.thread_count = 8;

	runtime_param.logging_param.include_config = 1;
	//runtime_param.logging_param.write_config = 1; // JSON dump
	runtime_param.logging_param.write_csv = 1;
	runtime_param.logging_param.export_interval = 0;
	runtime_param.logging_param.top_n_export = 1;


	config_ga_t config_ga = default_config(runtime_param);
	config_ga.selection_param.selection_method = selection_method_roulette;
	config_ga.population_param.reseed_bottom_N = 1;
	config_ga.crossover_param.crossover_method = crossover_method_two_point;

	config_ga.optimizer_param.convergence_window = 1000;
	config_ga.optimizer_param.convergence_moving_window_size = 100;
	config_ga.optimizer_param.max_iterations = 10000;
	config_ga.optimizer_param.max_mutations = 10;
	config_ga.optimizer_param.min_mutations = 1;
	config_ga.mutation_param.mutation_alpha = 10000;
	config_ga.mutation_param.mutation_beta = 0.1;



	for (int i = 0; i < repeats; i++) {
		printf("\n Run number: %d\n", i);

		//strcpy_s(runtime_param.fully_qualified_basename, 255, "C:/temp/GA\0");
		//printf("%s\n", runtime_param.fully_qualified_basename);
		//printf("%d\n", strlen(runtime_param.fully_qualified_basename));
		//Genetic_Algorithm(config_ga, runtime_param);

		Genetic_Algorithm(config_ga, runtime_param);
	}
	free_config_ga(&config_ga);

	return 0;
}