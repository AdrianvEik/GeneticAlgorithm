#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Helper/struct.h"
#include "../Helper/error_handling.h"
#include "../Multiprocessing/mp_solver_th.h"

void write_config(
    runtime_param_t runtime_param,
    config_ga_t config_ga
)
{
    int fully_qualified_basename_size = strlen(runtime_param.logging_param.fully_qualified_basename) + 1;
    char* filename_json = (char*)malloc(fully_qualified_basename_size + 5);

    if (filename_json == NULL) EXIT_MEM_ERROR();

    strcpy_s(filename_json, fully_qualified_basename_size, runtime_param.logging_param.fully_qualified_basename);
    strcat_s(filename_json, fully_qualified_basename_size + 5, ".json");

    FILE* fileptrconfig = fopen(filename_json, "w");
    if (fileptrconfig == NULL)
    {
        EXIT_WITH_ERROR("Cannot open file!", 1);
    }

    fprintf(fileptrconfig, "{\n");

    // Write runtime parameters
    fprintf(fileptrconfig, "\"runtime_param\": {\n");
    fprintf(fileptrconfig, "    \"task_count\": %d,\n", runtime_param.task_count);
    fprintf(fileptrconfig, "    \"thread_count\": %d,\n", runtime_param.thread_count);
    fprintf(fileptrconfig, "    \"zone_enable\": %d\n", runtime_param.zone_enable);
    fprintf(fileptrconfig, "},\n");

    // Write gene pool parameters for GA
    fprintf(fileptrconfig, "\"gene_pool_param\": {\n");
    fprintf(fileptrconfig, "    \"genes\": %d,\n", runtime_param.genes);
    fprintf(fileptrconfig, "    \"individuals\": %d,\n", runtime_param.individuals);
    fprintf(fileptrconfig, "    \"elitism\": %d\n", runtime_param.elitism);
    fprintf(fileptrconfig, "},\n");

    // Write selection parameters
    fprintf(fileptrconfig, "\"selection_param\": {\n");
    fprintf(fileptrconfig, "    \"selection_method\": %d,\n", config_ga.selection_param.selection_method);
    fprintf(fileptrconfig, "    \"selection_div_param\": %f,\n", config_ga.selection_param.selection_div_param);
    fprintf(fileptrconfig, "    \"selection_prob_param\": %f,\n", config_ga.selection_param.selection_prob_param);
    fprintf(fileptrconfig, "    \"selection_temp_param\": %f,\n", config_ga.selection_param.selection_temp_param);
    fprintf(fileptrconfig, "    \"selection_tournament_size\": %d\n", config_ga.selection_param.selection_tournament_size);
    fprintf(fileptrconfig, "},\n");

    // Write flatten parameters
    fprintf(fileptrconfig, "\"flatten_param\": {\n");
    fprintf(fileptrconfig, "    \"flatten_method\": %d,\n", config_ga.flatten_param.flatten_method);
    fprintf(fileptrconfig, "    \"flatten_alpha\": %f,\n", config_ga.flatten_param.flatten_alpha);
    fprintf(fileptrconfig, "    \"flatten_beta\": %f\n", config_ga.flatten_param.flatten_beta);
    fprintf(fileptrconfig, "},\n");

    // Write crossover parameters
    fprintf(fileptrconfig, "\"crossover_param\": {\n");
    fprintf(fileptrconfig, "    \"crossover_method\": %d,\n", config_ga.crossover_param.crossover_method);
    fprintf(fileptrconfig, "    \"crossover_prob\": %f\n", config_ga.crossover_param.crossover_prob);
    fprintf(fileptrconfig, "},\n");

    // Write mutation parameters
    fprintf(fileptrconfig, "\"mutation_param\": {\n");
    fprintf(fileptrconfig, "    \"mutation_method\": %d,\n", config_ga.mutation_param.mutation_method);
    fprintf(fileptrconfig, "    \"mutation_prob\": %f,\n", config_ga.mutation_param.mutation_prob);
    fprintf(fileptrconfig, "    \"mutation_rate\": %d\n", config_ga.mutation_param.mutation_rate);
    fprintf(fileptrconfig, "},\n");

    // Write fx parameters
    fprintf(fileptrconfig, "\"fx_param\": {\n");
    fprintf(fileptrconfig, "    \"fx_method\": %d,\n", config_ga.fx_param.fx_method);
    fprintf(fileptrconfig, "    \"fx_optim_mode\": %d\n", config_ga.fx_param.fx_optim_mode);
    fprintf(fileptrconfig, "},\n");

    // Write population parameters
    fprintf(fileptrconfig, "\"population_param\": {\n");
    fprintf(fileptrconfig, "    \"pop_sampling_type\": %d,\n", config_ga.population_param.sampling_type);
    fprintf(fileptrconfig, "    \"pop_sigma\": %d,\n", config_ga.population_param.sigma);
    //fprintf(fileptrconfig, "    \"pop_lower\": %d,\n", config_ga.population_param.lower);
    //fprintf(fileptrconfig, "    \"pop_upper\": %d\n", config_ga.population_param.upper);
    fprintf(fileptrconfig, "},\n");

    // Write optimizer parameters
    fprintf(fileptrconfig, "\"optimizer_param\": {\n");
    fprintf(fileptrconfig, "    \"convergence_moving_window_size\": %d,\n", config_ga.optimizer_param.convergence_moving_window_size);
    fprintf(fileptrconfig, "    \"min_mutations\": %d,\n", config_ga.optimizer_param.min_mutations);
    fprintf(fileptrconfig, "    \"max_mutations\": %d,\n", config_ga.optimizer_param.max_mutations);
    fprintf(fileptrconfig, "    \"mutation_factor\": %f,\n", config_ga.optimizer_param.mutation_factor);
    fprintf(fileptrconfig, "    \"max_iterations\": %d,\n", config_ga.optimizer_param.max_iterations);
    fprintf(fileptrconfig, "    \"convergence_threshold\": %f,\n", config_ga.optimizer_param.convergence_threshold);
    fprintf(fileptrconfig, "    \"convergence_window\": %d\n", config_ga.optimizer_param.convergence_window);
    fprintf(fileptrconfig, "}\n");

    fprintf(fileptrconfig, "}\n");

    fclose(fileptrconfig);
    free(filename_json);
}
