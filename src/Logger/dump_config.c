
#include "dump_config.h"

void write_config(
    runtime_param_t runtime_param,
    config_ga_t config_ga
)
{
    uint64_t fully_qualified_basename_size = strlen(runtime_param.logging_param.fully_qualified_basename) + 1;
    char* filename_json = (char*)malloc(fully_qualified_basename_size + 5);

    if (filename_json == NULL) EXIT_MEM_ERROR();

    strcpy_s(filename_json, fully_qualified_basename_size, runtime_param.logging_param.fully_qualified_basename);
    strcat_s(filename_json, fully_qualified_basename_size + 5, ".json");

    FILE* fileptrconfig = {0};
    if (fopen_s(&fileptrconfig, filename_json, "w")) {
        free(filename_json);
        EXIT_WITH_ERROR("Cannot open file!", 1);
    }
    

    fprintf_s(fileptrconfig, "{\n");

    // Write runtime parameters
    fprintf_s(fileptrconfig, "\"runtime_param\": {\n");
    fprintf_s(fileptrconfig, "    \"task_count\": %d,\n", runtime_param.task_count);
    fprintf_s(fileptrconfig, "    \"thread_count\": %d,\n", runtime_param.thread_count);
    fprintf_s(fileptrconfig, "    \"zone_enable\": %d\n", runtime_param.zone_enable);
    fprintf_s(fileptrconfig, "},\n");

    // Write gene pool parameters for GA
    fprintf_s(fileptrconfig, "\"gene_pool_param\": {\n");
    fprintf_s(fileptrconfig, "    \"genes\": %d,\n", runtime_param.genes);
    fprintf_s(fileptrconfig, "    \"individuals\": %d,\n", runtime_param.individuals);
    fprintf_s(fileptrconfig, "    \"elitism\": %d\n", runtime_param.elitism);
    fprintf_s(fileptrconfig, "},\n");

    // Write selection parameters
    fprintf_s(fileptrconfig, "\"selection_param\": {\n");
    fprintf_s(fileptrconfig, "    \"selection_method\": %d,\n", config_ga.selection_param.selection_method);
    fprintf_s(fileptrconfig, "    \"selection_div_param\": %f,\n", config_ga.selection_param.selection_div_param);
    fprintf_s(fileptrconfig, "    \"selection_prob_param\": %f,\n", config_ga.selection_param.selection_prob_param);
    fprintf_s(fileptrconfig, "    \"selection_temp_param\": %f,\n", config_ga.selection_param.selection_temp_param);
    fprintf_s(fileptrconfig, "    \"selection_tournament_size\": %d\n", config_ga.selection_param.selection_tournament_size);
    fprintf_s(fileptrconfig, "},\n");

    // Write flatten parameters
    fprintf_s(fileptrconfig, "\"flatten_param\": {\n");
    fprintf_s(fileptrconfig, "    \"flatten_method\": %d,\n", config_ga.flatten_param.flatten_method);
    fprintf_s(fileptrconfig, "    \"flatten_alpha\": %f,\n", config_ga.flatten_param.flatten_alpha);
    fprintf_s(fileptrconfig, "    \"flatten_beta\": %f\n", config_ga.flatten_param.flatten_beta);
    fprintf_s(fileptrconfig, "},\n");

    // Write crossover parameters
    fprintf_s(fileptrconfig, "\"crossover_param\": {\n");
    fprintf_s(fileptrconfig, "    \"crossover_method\": %d,\n", config_ga.crossover_param.crossover_method);
    fprintf_s(fileptrconfig, "    \"crossover_prob\": %f\n", config_ga.crossover_param.crossover_prob);
    fprintf_s(fileptrconfig, "},\n");

    // Write mutation parameters
    fprintf_s(fileptrconfig, "\"mutation_param\": {\n");
    fprintf_s(fileptrconfig, "    \"mutation_method\": %d,\n", config_ga.mutation_param.mutation_method);
    fprintf_s(fileptrconfig, "    \"mutation_prob\": %f,\n", config_ga.mutation_param.mutation_prob);
    fprintf_s(fileptrconfig, "},\n");

    // Write fx parameters
    fprintf_s(fileptrconfig, "\"fx_param\": {\n");
    fprintf_s(fileptrconfig, "    \"fx_method\": %d,\n", config_ga.fx_param.fx_method);
    fprintf_s(fileptrconfig, "    \"fx_optim_mode\": %d\n", config_ga.fx_param.fx_optim_mode);
    fprintf_s(fileptrconfig, "},\n");

    // Write population parameters
    fprintf_s(fileptrconfig, "\"population_param\": {\n");
    fprintf_s(fileptrconfig, "    \"pop_sampling_type\": %d,\n", config_ga.population_param.sampling_type);
    fprintf_s(fileptrconfig, "    \"pop_sigma\": %d,\n", config_ga.population_param.sigma);
    //fprintf_s(fileptrconfig, "    \"pop_lower\": %d,\n", config_ga.population_param.lower);
    //fprintf_s(fileptrconfig, "    \"pop_upper\": %d\n", config_ga.population_param.upper);
    fprintf_s(fileptrconfig, "},\n");

    // Write optimizer parameters
    fprintf_s(fileptrconfig, "\"optimizer_param\": {\n");
    fprintf_s(fileptrconfig, "    \"convergence_moving_window_size\": %d,\n", config_ga.optimizer_param.convergence_moving_window_size);
    fprintf_s(fileptrconfig, "    \"min_mutations\": %f,\n", config_ga.optimizer_param.min_mutations);
    fprintf_s(fileptrconfig, "    \"max_mutations\": %f,\n", config_ga.optimizer_param.max_mutations);
    fprintf_s(fileptrconfig, "    \"mutation_factor\": %f,\n", config_ga.optimizer_param.mutation_factor);
    fprintf_s(fileptrconfig, "    \"max_iterations\": %d,\n", config_ga.optimizer_param.max_iterations);
    fprintf_s(fileptrconfig, "    \"convergence_threshold\": %f,\n", config_ga.optimizer_param.convergence_threshold);
    fprintf_s(fileptrconfig, "    \"convergence_window\": %d\n", config_ga.optimizer_param.convergence_window);
    fprintf_s(fileptrconfig, "}\n");

    fprintf_s(fileptrconfig, "}\n");

    fclose(fileptrconfig);
    free(filename_json);
}
