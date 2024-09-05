
#include "../helper/struct.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Helper/multiprocessing.h"

void open_file(gene_pool_t gene_pool, thread_param_t* thread_param, char* log_file)
{
	int fully_qualified_basename_size = strlen(log_file)+1;
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

	strcpy_s(filename_csv, fully_qualified_basename_size, log_file);
	strcat_s(filename_csv, fully_qualified_basename_size+4, ".csv");

	strcpy_s(filename_bin, fully_qualified_basename_size, log_file);
	strcat_s(filename_bin, fully_qualified_basename_size+4, ".bin");

	strcpy_s(filename_json, fully_qualified_basename_size, log_file);
	strcat_s(filename_json, fully_qualified_basename_size+5, ".json");


	if(fopen_s (&(thread_param->fileptr), filename_bin, "wb") != 0 ||
	   fopen_s (&(thread_param->fileptrcsv), filename_csv, "w") != 0 ||
	   fopen_s (&(thread_param->fileptrconfig), filename_json, "w") != 0)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(thread_param->fileptrcsv, "iteration, individual, result, ");
	for (int i = 0; i < gene_pool.genes; i++)
	{
		fprintf(thread_param->fileptrcsv, "gene%d, ", i);
	}
	fprintf(thread_param->fileptrcsv, "\n");


}

void close_file(thread_param_t thread_param)
{
	fclose(thread_param.fileptr);
	fclose(thread_param.fileptrcsv);
	fclose(thread_param.fileptrconfig);
}

void write_param(gene_pool_t gene_pool, thread_param_t thread_param, int iteration)
{
	if (thread_param.fileptr == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fwrite(&iteration, sizeof(int), 1, thread_param.fileptr);

	int paramsetsize = sizeof(gene_pool.pop_param_double[0]);

	for (int i = 0; i < gene_pool.individuals; i++)
	{
		fwrite(&gene_pool.pop_result_set[gene_pool.sorted_indexes[i]], paramsetsize, 1, thread_param.fileptr);
	

		fprintf(thread_param.fileptrcsv, "%d, %d, %f, ", iteration, i, gene_pool.pop_result_set[gene_pool.sorted_indexes[i]]);
		for (int j = 0; j < gene_pool.genes; j++)
		{
			fwrite(&gene_pool.pop_param_double[gene_pool.sorted_indexes[i]][j], paramsetsize, 1, thread_param.fileptr);

			fprintf(thread_param.fileptrcsv, "%f, ", gene_pool.pop_param_double[gene_pool.sorted_indexes[i]][j]);
		}
		fprintf(thread_param.fileptrcsv, "\n");
	}
}

void write_thread_result(runtime_param_t run_param, config_ga_t config_ga, thread_param_t thread_param){
	if (thread_param.fileptrconfig == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	// Write runtime parameters as binary header
	fwrite(&run_param.max_iterations, sizeof(int), 1, thread_param.fileptrconfig);
	fwrite(&run_param.convergence_window, sizeof(int), 1, thread_param.fileptrconfig);
	fwrite(&run_param.convergence_threshold, sizeof(double), 1, thread_param.fileptrconfig);

	// Write GA parameters as binary header
	fwrite(&config_ga.selection_param.selection_method, sizeof(int), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.selection_param.selection_div_param, sizeof(double), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.selection_param.selection_prob_param, sizeof(double), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.selection_param.selection_temp_param, sizeof(double), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.selection_param.selection_tournament_size, sizeof(int), 1, thread_param.fileptrconfig);

	fwrite(&config_ga.flatten_param.flatten_method, sizeof(int), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.flatten_param.flatten_factor, sizeof(double), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.flatten_param.flatten_bias, sizeof(double), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.flatten_param.flatten_optim_mode, sizeof(int), 1, thread_param.fileptrconfig);

	fwrite(&config_ga.crossover_param.crossover_method, sizeof(int), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.crossover_param.crossover_prob, sizeof(double), 1, thread_param.fileptrconfig);

	fwrite(&config_ga.mutation_param.mutation_method, sizeof(int), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.mutation_param.mutation_prob, sizeof(double), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.mutation_param.mutation_rate, sizeof(int), 1, thread_param.fileptrconfig);

	fwrite(&config_ga.fx_param.fx_method, sizeof(int), 1, thread_param.fileptrconfig);
	fwrite(&config_ga.fx_param.fx_optim_mode, sizeof(int), 1, thread_param.fileptrconfig);

	// Write thread parameters
	fwrite(&thread_param.thread_id, sizeof(int), 1, thread_param.fileptrconfig);

	// Write task list
	for (int i = 0; i < run_param.task_count; i++) {
		fwrite(&thread_param.task_list[i].thread_id, sizeof(int), 1, thread_param.fileptrconfig);
		fwrite(&thread_param.task_list[i].task_id, sizeof(int), 1, thread_param.fileptrconfig);
		fwrite(&thread_param.task_list[i].status, sizeof(int), 1, thread_param.fileptrconfig);
		fwrite(thread_param.task_list[i].lower, sizeof(double), run_param.genes, thread_param.fileptrconfig);
		fwrite(thread_param.task_list[i].upper, sizeof(double), run_param.genes, thread_param.fileptrconfig);
		fwrite(thread_param.task_list[i].paramset, sizeof(double), run_param.genes, thread_param.fileptrconfig);
		fwrite(&thread_param.task_list[i].result, sizeof(double), 1, thread_param.fileptrconfig);
	}

}

void write_config(gene_pool_t gene_pool, thread_param_t thread_param)
{

	if (thread_param.fileptrconfig == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(thread_param.fileptrconfig,  "{\n");

	// Write runtime parameters
	fprintf(thread_param.fileptrconfig, "\"runtime_param\": {\n");
	fprintf(thread_param.fileptrconfig, "\"max_iterations\":%d,\n", thread_param.runtime_param.max_iterations);
	fprintf(thread_param.fileptrconfig, "\"convergence_window\":%d,\n", thread_param.runtime_param.convergence_window);
	fprintf(thread_param.fileptrconfig, "\"convergence_threshold\":%f\n", thread_param.runtime_param.convergence_threshold);
	fprintf(thread_param.fileptrconfig, "},\n");

	// Config GA param
	fprintf(thread_param.fileptrconfig, "\"gene_pool_param\":{\n");
	fprintf(thread_param.fileptrconfig, "\"genes\":%d,\n", gene_pool.genes);
	fprintf(thread_param.fileptrconfig, "\"individuals\":%d,\n", gene_pool.individuals);
	fprintf(thread_param.fileptrconfig, "\"elitism\":%d\n", gene_pool.elitism);
	fprintf(thread_param.fileptrconfig, "},\n");

	// selection param
	fprintf(thread_param.fileptrconfig, "\"selection_param\":{\n");
	fprintf(thread_param.fileptrconfig, "\"selection_method\":%d\n,", thread_param.config_ga.selection_param.selection_method);
	fprintf(thread_param.fileptrconfig, "\"selection_div_param\":%f\n,", thread_param.config_ga.selection_param.selection_div_param);
	fprintf(thread_param.fileptrconfig, "\"selection_prob_param\":%f\n,", thread_param.config_ga.selection_param.selection_prob_param);
	fprintf(thread_param.fileptrconfig, "\"selection_temp_param\":%f\n,", thread_param.config_ga.selection_param.selection_temp_param);
	fprintf(thread_param.fileptrconfig, "\"selection_tournament_size\":%d\n", thread_param.config_ga.selection_param.selection_tournament_size);
	fprintf(thread_param.fileptrconfig, "},\n");

	// flattem param
	fprintf(thread_param.fileptrconfig, "\"flatten_param\":{\n");
	fprintf(thread_param.fileptrconfig, "\"flatten_method\":%d\n,", thread_param.config_ga.flatten_param.flatten_method);
	fprintf(thread_param.fileptrconfig, "\"flatten_factor\":%f\n,", thread_param.config_ga.flatten_param.flatten_factor);
	fprintf(thread_param.fileptrconfig, "\"flatten_bias\":%f\n,", thread_param.config_ga.flatten_param.flatten_bias);
	fprintf(thread_param.fileptrconfig, "\"flatten_optim_mode\":%d\n", thread_param.config_ga.flatten_param.flatten_optim_mode);
	fprintf(thread_param.fileptrconfig, "},\n");

	// crossover param
	fprintf(thread_param.fileptrconfig, "\"crossover_param\":{\n");
	fprintf(thread_param.fileptrconfig, "\"crossover_method\":%d,\n", thread_param.config_ga.crossover_param.crossover_method);
	fprintf(thread_param.fileptrconfig, "\"crossover_prob\":%f\n", thread_param.config_ga.crossover_param.crossover_prob);
	fprintf(thread_param.fileptrconfig, "},\n");


	// mutation param
	fprintf(thread_param.fileptrconfig, "\"mutation_param\":{\n");
	fprintf(thread_param.fileptrconfig, "\"mutation_method\":%d,\n", thread_param.config_ga.mutation_param.mutation_method);
	fprintf(thread_param.fileptrconfig, "\"mutation_prob\":%f,\n", thread_param.config_ga.mutation_param.mutation_prob);
	fprintf(thread_param.fileptrconfig, "\"mutation_rate\":%d\n", thread_param.config_ga.mutation_param.mutation_rate);
	fprintf(thread_param.fileptrconfig, "},\n");

	// fx param
	fprintf(thread_param.fileptrconfig, "\"fx_param\":{\n");
	fprintf(thread_param.fileptrconfig, "\"fx_method\":%d,\n", thread_param.config_ga.fx_param.fx_method);
	fprintf(thread_param.fileptrconfig, "\"fx_optim_mode\":%d,\n", thread_param.config_ga.fx_param.fx_optim_mode);
	fprintf(thread_param.fileptrconfig, "\"fx_lower\":%f,\n", thread_param.config_ga.fx_param.lower);
	fprintf(thread_param.fileptrconfig, "\"fx_upper\":%f\n", thread_param.config_ga.fx_param.upper);
	fprintf(thread_param.fileptrconfig, "}\n");

	fprintf(thread_param.fileptrconfig, "}\n");
}