
#include "../Helper/struct.h"
#include "../Helper/Helper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Helper/multiprocessing.h"
#include "../Optimisation/Optimizer.h"

static inline void copy_to_bin_buffer(task_result_t* task_result, void* data, int size) {
	if (task_result->bin_single_entry_length < size + task_result->bin_position) {
		printf("Buffer overflow\n");
	}
	memcpy(task_result->bin_buffer + task_result->bin_position, data, size);
	task_result->bin_position += size;
}

static inline void copy_to_csv_buffer(task_result_t* task_result, void* data, int size) {
	if (task_result->csv_single_entry_length < size + task_result->csv_position) {
		printf("Buffer overflow\n");
	}
	memcpy(task_result->csv_buffer + task_result->csv_position, data, size);
	task_result->bin_position += size;
}

void copy_task_result(task_result_t* task_result, task_result_t* source) {
    memcpy(task_result->bin_buffer, source->bin_buffer, source->bin_position);
	if (source->csv_position > 0) {
		memcpy(task_result->csv_buffer, source->csv_buffer, source->csv_position);
	}
    task_result->bin_position = source->bin_position;
    task_result->csv_position = source->csv_position;
    task_result->result = source->result;
}

void open_file(task_result_queue_t* task_result_queue)
{
	int fully_qualified_basename_size = strlen(task_result_queue->runtime_param.logging_param.fully_qualified_basename)+1;
	char* filename_csv = malloc(fully_qualified_basename_size + 4);
	char* filename_bin = malloc(fully_qualified_basename_size + 4);

	if (fully_qualified_basename_size == 0) {
		printf("File name is empty");
		exit(1);
	}

	if (filename_csv == NULL || filename_bin == NULL) {
		printf("Memory allocation failed");
		exit(255);
	}

	strcpy_s(filename_csv, fully_qualified_basename_size, task_result_queue->runtime_param.logging_param.fully_qualified_basename);
	strcat_s(filename_csv, fully_qualified_basename_size+4, ".csv");

	strcpy_s(filename_bin, fully_qualified_basename_size, task_result_queue->runtime_param.logging_param.fully_qualified_basename);
	strcat_s(filename_bin, fully_qualified_basename_size+4, ".bin");


	if(fopen_s (&(task_result_queue->fileptr), filename_bin, "wb") != 0 ||
	   fopen_s (&(task_result_queue->fileptrcsv), filename_csv, "w") != 0)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	//struct task_result_s {
//	int task_type; // 0: GA, 1: kill
//	int iteration;
//	int task_id;
//	int individual_id;
//	int position;
// 	double result;
//	double* lower;
//	double* upper;
//	double* paramset;
//	int* config_int;
//	double* config_double;
//};
	fprintf(task_result_queue->fileptrcsv, "iteration;task_id;individual_id;position;result;");
	for (int i = 0; i < task_result_queue->runtime_param.genes; i++)
	{
		fprintf(task_result_queue->fileptrcsv, "lower%d;", i);
        fprintf(task_result_queue->fileptrcsv, "upper%d;", i);
		fprintf(task_result_queue->fileptrcsv, "gene%d;", i);
	}
	if (task_result_queue->runtime_param.logging_param.include_config == 1) {
		for (int i = 0; i < task_result_queue->runtime_param.logging_param.config_int_count; i++)
		{
			fprintf(task_result_queue->fileptrcsv, "config_int%d;", i);
		}
		for (int i = 0; i < task_result_queue->runtime_param.logging_param.config_double_count; i++)
		{
			fprintf(task_result_queue->fileptrcsv, "config_double%d;", i);
		}
	}
	
	fprintf(task_result_queue->fileptrcsv, "\n");

}

void close_file(task_result_queue_t* task_result_queue)
{
	fclose(task_result_queue->fileptr);
	fclose(task_result_queue->fileptrcsv);
}

void report_task(task_queue_t* task_queue, task_param_t* task, adaptive_memory_t* adaptive_memory, thread_param_t* thread_param, gene_pool_t* gene_pool, int best_result, int time_spent_us) {
	task_result_t task_result;
	int log_top_n;

	if (best_result == 1) {
		task_result.task_type = BEST_RESULT_TASK;
        log_top_n = 1;
		task_result.result = gene_pool->pop_result_set[gene_pool->sorted_indexes[gene_pool->individuals - 1]];
    }
    else {
        task_result.task_type = LOG_TASK;
		log_top_n = thread_param->runtime_param.logging_param.top_n_export;
    }

	init_task_result(task_queue->task_result_queue, &task_result, log_top_n);


	for (int individual = 0; individual < log_top_n; individual++) {

		int individual_id = gene_pool->sorted_indexes[gene_pool->individuals - individual - 1];
		double result = gene_pool->pop_result_set[individual_id]; // result and invert if optim mode is minimisation
		
		copy_to_bin_buffer(&task_result, &adaptive_memory->iteration_counter, sizeof(int));
		copy_to_bin_buffer(&task_result, &task->task_id, sizeof(int));
		copy_to_bin_buffer(&task_result, &individual_id, sizeof(int));
		copy_to_bin_buffer(&task_result, &individual, sizeof(int)); // position
		copy_to_bin_buffer(&task_result, &result, sizeof(double));
		copy_to_bin_buffer(&task_result, task->lower, sizeof(double) * thread_param->runtime_param.genes);
		copy_to_bin_buffer(&task_result, task->upper, sizeof(double) * thread_param->runtime_param.genes);
		copy_to_bin_buffer(&task_result, gene_pool->pop_param_double[individual_id], sizeof(double) * thread_param->runtime_param.genes);

		if (thread_param->runtime_param.logging_param.write_csv == 1) {
			task_result.csv_position += snprintf(
				task_result.csv_buffer + task_result.csv_position,
				task_queue->task_result_queue->csv_single_entry_length - task_result.csv_position,
				"%d;%d;%d;%d;%e;",
				adaptive_memory->iteration_counter,
				task->task_id,
				individual_id,
                individual, // position
                result 
			);
			for (int i = 0; i < thread_param->runtime_param.genes; i++)
			{
				task_result.csv_position += snprintf(
					task_result.csv_buffer + task_result.csv_position,
					task_queue->task_result_queue->csv_single_entry_length - task_result.csv_position,
					"%e;%e;%e;",
					task->lower[i],
					task->upper[i],
					gene_pool->pop_param_double[individual_id][i]
					);
			}
		}
		if (thread_param->runtime_param.logging_param.include_config == 1) {
			//task_result.config_int[0] = task->config_ga.mutation_param.mutation_rate;
			//task_result.config_double[0] = adaptive_memory->computed_mutation;
			//task_result.config_double[1] = adaptive_memory->convergence_moving_window;
			copy_to_bin_buffer(&task_result, &task->config_ga.mutation_param.mutation_rate, sizeof(int));
			copy_to_bin_buffer(&task_result, &adaptive_memory->computed_mutation, sizeof(double));
			copy_to_bin_buffer(&task_result, &adaptive_memory->convergence_moving_window, sizeof(double));

			if (thread_param->runtime_param.logging_param.write_csv == 1) {
				task_result.csv_position += snprintf(
					task_result.csv_buffer + task_result.csv_position,
					task_queue->task_result_queue->csv_single_entry_length - task_result.csv_position,
					"%d;%e;%e;",
                    task->config_ga.mutation_param.mutation_rate,
					adaptive_memory->computed_mutation,
                    adaptive_memory->convergence_moving_window
				);
			}
		}
		if (thread_param->runtime_param.logging_param.write_csv == 1) {
			task_result.csv_position += snprintf(
				task_result.csv_buffer + task_result.csv_position,
				task_queue->task_result_queue->csv_single_entry_length - task_result.csv_position,
				"\n"
			);
		}
	}

	add_result(thread_param->task_queue->task_result_queue, &task_result);
}

void write_file_buffer(task_result_queue_t* task_result_queue, task_result_t* task_result) {

	fwrite(task_result->bin_buffer, task_result->bin_position, 1, task_result_queue->fileptr);
	//task_result->bin_position = 0;
	//task_result->bin_buffer[0] = '\0';
	

    if (task_result_queue->runtime_param.logging_param.write_csv == 1) {
        //fprintf(task_result_queue->fileptrcsv, "%s", task_result->csv_buffer);
        fwrite(task_result->csv_buffer, task_result->csv_position, 1, task_result_queue->fileptrcsv);
        //task_result->csv_position = 0;
        //task_result->csv_buffer[0] = '\0';
    }
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