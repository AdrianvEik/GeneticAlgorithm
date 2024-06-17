
#include "../helper/struct.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

FILE* fileptr = NULL;
FILE* fileptrcsv = NULL;
FILE* fileptrconfig = NULL;

void open_file(gene_pool_t gene_pool, runtime_param_t runtime_param)
{
	int fully_qualified_basename_size = strlen(runtime_param.fully_qualified_basename)+1;
	char* filename_csv = malloc(fully_qualified_basename_size + 4);
	char* filename_bin = malloc(fully_qualified_basename_size + 4);
	char* filename_json = malloc(fully_qualified_basename_size + 5);

	if (fully_qualified_basename_size == 0) {
		printf("File name is empty");
		exit(1);
	}

	if (filename_csv == NULL || filename_bin == NULL || filename_json == NULL) {
		printf("Memory allocation failed");
		exit(255);
	}

	strcpy_s(filename_csv, fully_qualified_basename_size, runtime_param.fully_qualified_basename);
	strcat_s(filename_csv, fully_qualified_basename_size+4, ".csv");

	strcpy_s(filename_bin, fully_qualified_basename_size, runtime_param.fully_qualified_basename);
	strcat_s(filename_bin, fully_qualified_basename_size+4, ".bin");

	strcpy_s(filename_json, fully_qualified_basename_size, runtime_param.fully_qualified_basename);
	strcat_s(filename_json, fully_qualified_basename_size+5, ".json");

	if(fopen_s (&fileptr, filename_bin, "wb") != 0 ||
	   fopen_s (&fileptrcsv, filename_csv, "w") != 0 ||
	   fopen_s (&fileptrconfig, filename_json, "w") != 0)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(fileptrcsv, "iteration, individual, result, ");
	for (int i = 0; i < gene_pool.genes; i++)
	{
		fprintf(fileptrcsv, "gene%d, ", i);
	}
	fprintf(fileptrcsv, "\n");


}

void close_file()
{
	fclose(fileptr);
	fclose(fileptrcsv);
	fclose(fileptrconfig);
}

void write_param(gene_pool_t gene_pool, int iteration)
{
	if (fileptr == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fwrite(&iteration, sizeof(int), 1, fileptr);
	fwrite(&gene_pool.pop_result_set, sizeof(gene_pool.pop_result_set), 1, fileptr);

	int paramsetsize = sizeof(gene_pool.pop_param_double[0]);

	for (int i = 0; i < gene_pool.individuals; i++)
	{
		fwrite(&gene_pool.pop_param_double[i], paramsetsize, 1, fileptr);

		fprintf(fileptrcsv, "%d, %d, %f, ", iteration, i, gene_pool.pop_result_set[gene_pool.sorted_indexes[i]]);
		for (int j = 0; j < gene_pool.genes; j++)
		{
			fprintf(fileptrcsv, "%f, ", gene_pool.pop_param_double[i][j]);
		}
		fprintf(fileptrcsv, "\n");
	}
}

void write_config(gene_pool_t gene_pool, runtime_param_t run_param, config_ga_t config_ga)
{

	if (fileptrconfig == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(fileptrconfig,  "{\n");

	// Write runtime parameters
	fprintf(fileptrconfig, "\"runtime_param\": {\n");
	fprintf(fileptrconfig, "\"max_iterations\":%d,\n", run_param.max_iterations);
	fprintf(fileptrconfig, "\"convergence_window\":%d,\n", run_param.convergence_window);
	fprintf(fileptrconfig, "\"convergence_threshold\":%f\n", run_param.convergence_threshold);
	fprintf(fileptrconfig, "},\n");

	// Config GA param
	fprintf(fileptrconfig, "\"gene_pool_param\":{\n");
	fprintf(fileptrconfig, "\"genes\":%d,\n", gene_pool.genes);
	fprintf(fileptrconfig, "\"individuals\":%d,\n", gene_pool.individuals);
	fprintf(fileptrconfig, "\"elitism\":%d\n", gene_pool.elitism);
	fprintf(fileptrconfig, "},\n");

	// selection param
	fprintf(fileptrconfig, "\"selection_param\":{\n");
	fprintf(fileptrconfig, "\"selection_method\":%d\n,", config_ga.selection_param.selection_method);
	fprintf(fileptrconfig, "\"selection_div_param\":%f\n,", config_ga.selection_param.selection_div_param);
	fprintf(fileptrconfig, "\"selection_prob_param\":%f\n,", config_ga.selection_param.selection_prob_param);
	fprintf(fileptrconfig, "\"selection_temp_param\":%f\n,", config_ga.selection_param.selection_temp_param);
	fprintf(fileptrconfig, "\"selection_tournament_size\":%d\n", config_ga.selection_param.selection_tournament_size);
	fprintf(fileptrconfig, "},\n");

	// flattem param
	fprintf(fileptrconfig, "\"flatten_param\":{\n");
	fprintf(fileptrconfig, "\"flatten_method\":%d\n,", config_ga.flatten_param.flatten_method);
	fprintf(fileptrconfig, "\"flatten_factor\":%f\n,", config_ga.flatten_param.flatten_factor);
	fprintf(fileptrconfig, "\"flatten_bias\":%f\n,", config_ga.flatten_param.flatten_bias);
	fprintf(fileptrconfig, "\"flatten_optim_mode\":%d\n", config_ga.flatten_param.flatten_optim_mode);
	fprintf(fileptrconfig, "},\n");

	// crossover param
	fprintf(fileptrconfig, "\"crossover_param\":{\n");
	fprintf(fileptrconfig, "\"crossover_method\":%d,\n", config_ga.crossover_param.crossover_method);
	fprintf(fileptrconfig, "\"crossover_prob\":%f\n", config_ga.crossover_param.crossover_prob);
	fprintf(fileptrconfig, "},\n");


	// mutation param
	fprintf(fileptrconfig, "\"mutation_param\":{\n");
	fprintf(fileptrconfig, "\"mutation_method\":%d,\n", config_ga.mutation_param.mutation_method);
	fprintf(fileptrconfig, "\"mutation_prob\":%f,\n", config_ga.mutation_param.mutation_prob);
	fprintf(fileptrconfig, "\"mutation_rate\":%d\n", config_ga.mutation_param.mutation_rate);
	fprintf(fileptrconfig, "},\n");

	// fx param
	fprintf(fileptrconfig, "\"fx_param\":{\n");
	fprintf(fileptrconfig, "\"fx_method\":%d,\n", config_ga.fx_param.fx_method);
	fprintf(fileptrconfig, "\"fx_optim_mode\":%d,\n", config_ga.fx_param.fx_optim_mode);
	fprintf(fileptrconfig, "\"bin2double_factor\":%f,\n", config_ga.fx_param.lower);
	fprintf(fileptrconfig, "\"bin2double_bias\":%f\n", config_ga.fx_param.upper);
	fprintf(fileptrconfig, "}\n");

	fprintf(fileptrconfig, "}\n");
}