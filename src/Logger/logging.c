
#include "../helper/struct.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Helper/multiprocessing.h"

void open_file(task_result_queue_t* task_result_queue)
{
	int fully_qualified_basename_size = strlen(task_result_queue->runtime_param.fully_qualified_basename)+1;
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

	strcpy_s(filename_csv, fully_qualified_basename_size, task_result_queue->runtime_param.fully_qualified_basename);
	strcat_s(filename_csv, fully_qualified_basename_size+4, ".csv");

	strcpy_s(filename_bin, fully_qualified_basename_size, task_result_queue->runtime_param.fully_qualified_basename);
	strcat_s(filename_bin, fully_qualified_basename_size+4, ".bin");

	strcpy_s(filename_json, fully_qualified_basename_size, task_result_queue->runtime_param.fully_qualified_basename);
	strcat_s(filename_json, fully_qualified_basename_size+5, ".json");


	if(fopen_s (&(task_result_queue->fileptr), filename_bin, "wb") != 0 ||
	   fopen_s (&(task_result_queue->fileptrcsv), filename_csv, "w") != 0 ||
	   fopen_s (&(task_result_queue->fileptrconfig), filename_json, "w") != 0)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(task_result_queue->fileptrcsv, "iteration;result;");
	for (int i = 0; i < task_result_queue->runtime_param.genes; i++)
	{
		fprintf(task_result_queue->fileptrcsv, "gene%d;", i);
	}
	fprintf(task_result_queue->fileptrcsv, "\n");


}

void close_file(task_result_queue_t* task_result_queue)
{
	fclose(task_result_queue->fileptr);
	fclose(task_result_queue->fileptrcsv);
	fclose(task_result_queue->fileptrconfig);
}

void write_param(task_result_queue_t task_result_queue, task_result_t task_result)
{
	if (task_result_queue.fileptr == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fwrite(&task_result.iterations, sizeof(int), 1, task_result_queue.fileptr);

	int paramsetsize = sizeof(task_result.paramset[0]);
	fwrite(&task_result.paramset, paramsetsize, 1, task_result_queue.fileptr);

	fprintf(task_result_queue.fileptrcsv, "%d;%d;", &task_result.iterations, &task_result.result);
    for (int i = 0; i < task_result_queue.runtime_param.genes; i++)
    {
        fprintf(task_result_queue.fileptrcsv, "%f;", task_result.paramset[i]);
    }

	fprintf(task_result_queue.fileptrcsv, "\n");
}

//void write_thread_result(runtime_param_t run_param, config_ga_t config_ga, thread_param_t thread_param){
//	if (thread_param.fileptrconfig == NULL)
//	{
//		printf("Error opening file!\n");
//		exit(1);
//	}
//	// Write runtime parameters as binary header
//	fwrite(&run_param.max_iterations, sizeof(int), 1, thread_param.fileptrconfig);
//	fwrite(&run_param.convergence_window, sizeof(int), 1, thread_param.fileptrconfig);
//	fwrite(&run_param.convergence_threshold, sizeof(double), 1, thread_param.fileptrconfig);
//
//	// Write GA parameters as binary header
//	fwrite(&config_ga.selection_param.selection_method, sizeof(int), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.selection_param.selection_div_param, sizeof(double), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.selection_param.selection_prob_param, sizeof(double), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.selection_param.selection_temp_param, sizeof(double), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.selection_param.selection_tournament_size, sizeof(int), 1, thread_param.fileptrconfig);
//
//	fwrite(&config_ga.flatten_param.flatten_method, sizeof(int), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.flatten_param.flatten_factor, sizeof(double), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.flatten_param.flatten_bias, sizeof(double), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.flatten_param.flatten_optim_mode, sizeof(int), 1, thread_param.fileptrconfig);
//
//	fwrite(&config_ga.crossover_param.crossover_method, sizeof(int), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.crossover_param.crossover_prob, sizeof(double), 1, thread_param.fileptrconfig);
//
//	fwrite(&config_ga.mutation_param.mutation_method, sizeof(int), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.mutation_param.mutation_prob, sizeof(double), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.mutation_param.mutation_rate, sizeof(int), 1, thread_param.fileptrconfig);
//
//	fwrite(&config_ga.fx_param.fx_method, sizeof(int), 1, thread_param.fileptrconfig);
//	fwrite(&config_ga.fx_param.fx_optim_mode, sizeof(int), 1, thread_param.fileptrconfig);
//
//	// Write thread parameters
//	fwrite(&thread_param.thread_id, sizeof(int), 1, thread_param.fileptrconfig);
//
//	// Write task list
//	for (int i = 0; i < run_param.task_count; i++) {
//		fwrite(&thread_param.task_list[i].thread_id, sizeof(int), 1, thread_param.fileptrconfig);
//		fwrite(&thread_param.task_list[i].task_id, sizeof(int), 1, thread_param.fileptrconfig);
//		fwrite(&thread_param.task_list[i].status, sizeof(int), 1, thread_param.fileptrconfig);
//		fwrite(thread_param.task_list[i].lower, sizeof(double), run_param.genes, thread_param.fileptrconfig);
//		fwrite(thread_param.task_list[i].upper, sizeof(double), run_param.genes, thread_param.fileptrconfig);
//		fwrite(thread_param.task_list[i].paramset, sizeof(double), run_param.genes, thread_param.fileptrconfig);
//		fwrite(&thread_param.task_list[i].result, sizeof(double), 1, thread_param.fileptrconfig);
//	}
//
//}
//
//void write_config(gene_pool_t gene_pool, thread_param_t thread_param)
//{
//
//	if (thread_param.fileptrconfig == NULL)
//	{
//		printf("Error opening file!\n");
//		exit(1);
//	}
//	fprintf(thread_param.fileptrconfig,  "{\n");
//
//	// Write runtime parameters
//	fprintf(thread_param.fileptrconfig, "\"runtime_param\": {\n");
//	fprintf(thread_param.fileptrconfig, "\"max_iterations\":%d,\n", thread_param.runtime_param.max_iterations);
//	fprintf(thread_param.fileptrconfig, "\"convergence_window\":%d,\n", thread_param.runtime_param.convergence_window);
//	fprintf(thread_param.fileptrconfig, "\"convergence_threshold\":%f\n", thread_param.runtime_param.convergence_threshold);
//	fprintf(thread_param.fileptrconfig, "},\n");
//
//	// Config GA param
//	fprintf(thread_param.fileptrconfig, "\"gene_pool_param\":{\n");
//	fprintf(thread_param.fileptrconfig, "\"genes\":%d,\n", gene_pool.genes);
//	fprintf(thread_param.fileptrconfig, "\"individuals\":%d,\n", gene_pool.individuals);
//	fprintf(thread_param.fileptrconfig, "\"elitism\":%d\n", gene_pool.elitism);
//	fprintf(thread_param.fileptrconfig, "},\n");
//
//	// selection param
//	fprintf(thread_param.fileptrconfig, "\"selection_param\":{\n");
//	fprintf(thread_param.fileptrconfig, "\"selection_method\":%d\n,", thread_param.config_ga.selection_param.selection_method);
//	fprintf(thread_param.fileptrconfig, "\"selection_div_param\":%f\n,", thread_param.config_ga.selection_param.selection_div_param);
//	fprintf(thread_param.fileptrconfig, "\"selection_prob_param\":%f\n,", thread_param.config_ga.selection_param.selection_prob_param);
//	fprintf(thread_param.fileptrconfig, "\"selection_temp_param\":%f\n,", thread_param.config_ga.selection_param.selection_temp_param);
//	fprintf(thread_param.fileptrconfig, "\"selection_tournament_size\":%d\n", thread_param.config_ga.selection_param.selection_tournament_size);
//	fprintf(thread_param.fileptrconfig, "},\n");
//
//	// flattem param
//	fprintf(thread_param.fileptrconfig, "\"flatten_param\":{\n");
//	fprintf(thread_param.fileptrconfig, "\"flatten_method\":%d\n,", thread_param.config_ga.flatten_param.flatten_method);
//	fprintf(thread_param.fileptrconfig, "\"flatten_factor\":%f\n,", thread_param.config_ga.flatten_param.flatten_factor);
//	fprintf(thread_param.fileptrconfig, "\"flatten_bias\":%f\n,", thread_param.config_ga.flatten_param.flatten_bias);
//	fprintf(thread_param.fileptrconfig, "\"flatten_optim_mode\":%d\n", thread_param.config_ga.flatten_param.flatten_optim_mode);
//	fprintf(thread_param.fileptrconfig, "},\n");
//
//	// crossover param
//	fprintf(thread_param.fileptrconfig, "\"crossover_param\":{\n");
//	fprintf(thread_param.fileptrconfig, "\"crossover_method\":%d,\n", thread_param.config_ga.crossover_param.crossover_method);
//	fprintf(thread_param.fileptrconfig, "\"crossover_prob\":%f\n", thread_param.config_ga.crossover_param.crossover_prob);
//	fprintf(thread_param.fileptrconfig, "},\n");
//
//
//	// mutation param
//	fprintf(thread_param.fileptrconfig, "\"mutation_param\":{\n");
//	fprintf(thread_param.fileptrconfig, "\"mutation_method\":%d,\n", thread_param.config_ga.mutation_param.mutation_method);
//	fprintf(thread_param.fileptrconfig, "\"mutation_prob\":%f,\n", thread_param.config_ga.mutation_param.mutation_prob);
//	fprintf(thread_param.fileptrconfig, "\"mutation_rate\":%d\n", thread_param.config_ga.mutation_param.mutation_rate);
//	fprintf(thread_param.fileptrconfig, "},\n");
//
//	// fx param
//	fprintf(thread_param.fileptrconfig, "\"fx_param\":{\n");
//	fprintf(thread_param.fileptrconfig, "\"fx_method\":%d,\n", thread_param.config_ga.fx_param.fx_method);
//	fprintf(thread_param.fileptrconfig, "\"fx_optim_mode\":%d,\n", thread_param.config_ga.fx_param.fx_optim_mode);
//	fprintf(thread_param.fileptrconfig, "},\n");
//
//	// pop_param
//	fprintf(thread_param.fileptrconfig, "\"population_param\":{\n");
//	fprintf(thread_param.fileptrconfig, "\"pop_sampling_type\":%d,\n", thread_param.config_ga.population_param.sampling_type);
//	fprintf(thread_param.fileptrconfig, "\"pop_sigma\":%d,\n", thread_param.config_ga.population_param.sigma);
//	fprintf(thread_param.fileptrconfig, "\"pop_lower\":%f,\n", thread_param.config_ga.population_param.lower);
//	fprintf(thread_param.fileptrconfig, "\"pop_upper\":%f\n", thread_param.config_ga.population_param.upper);
//	fprintf(thread_param.fileptrconfig, "}\n");
//	fprintf(thread_param.fileptrconfig, "}\n");
//}