
#include "logging.h"

static inline void copy_to_bin_buffer(task_result_t* task_result, void* data, int size) {
	if (task_result->bin_buffer_length < size + task_result->bin_position) {
		//printf("Buffer overflow\n");
        EXIT_WITH_ERROR("Binary buffer overflow", 1);
	}
	if (memcpy_s(task_result->bin_buffer + task_result->bin_position, size ,data, size)) EXIT_MEM_ERROR();
	
	task_result->bin_position += size;
}

void copy_task_result(task_result_t* task_result, task_result_t* source) {
    if (memcpy_s(task_result->bin_buffer, source->bin_position, source->bin_buffer, source->bin_position)) EXIT_MEM_ERROR();
	if (source->csv_position > 0) {
		if(memcpy_s(task_result->csv_buffer, source->csv_position, source->csv_buffer, source->csv_position)) EXIT_MEM_ERROR();
	}
    task_result->bin_position = source->bin_position;
    task_result->csv_position = source->csv_position;
    task_result->result = source->result;
}

void open_file(task_result_queue_t* task_result_queue)
{

	uint64_t fully_qualified_basename_size = strlen(task_result_queue->runtime_param.logging_param.fully_qualified_basename)+1;
	char* filename_csv = malloc(fully_qualified_basename_size + 4);
	char* filename_bin = malloc(fully_qualified_basename_size + 4);

	if (fully_qualified_basename_size == 0) {
		free(filename_csv);
		free(filename_bin);
		EXIT_WITH_ERROR("Empty file name", 1);
	}

	if (filename_csv == NULL || filename_bin == NULL) {
		free(filename_csv);
		free(filename_bin);
		EXIT_MEM_ERROR();
	}

	strcpy_s(filename_csv, fully_qualified_basename_size, task_result_queue->runtime_param.logging_param.fully_qualified_basename);
	strcat_s(filename_csv, fully_qualified_basename_size+4, ".csv");

	strcpy_s(filename_bin, fully_qualified_basename_size, task_result_queue->runtime_param.logging_param.fully_qualified_basename);
	strcat_s(filename_bin, fully_qualified_basename_size+4, ".bin");

	if (task_result_queue->runtime_param.logging_param.write_bin == 1) {
		if (fopen_s(&(task_result_queue->fileptr), filename_bin, "wb")) {
			free(filename_csv);
            free(filename_bin);
            EXIT_WITH_ERROR("Cannot open file!", 1);
		}
    }

	if (task_result_queue->runtime_param.logging_param.write_csv == 1) {
		if (fopen_s(&(task_result_queue->fileptrcsv), filename_csv, "w")) {
			free(filename_csv);
			free(filename_bin);
			EXIT_WITH_ERROR("Cannot open file!", 1);
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
		fprintf_s(task_result_queue->fileptrcsv, "iteration;task_id;individual_id;position;result;");
		for (int i = 0; i < task_result_queue->runtime_param.genes; i++)
		{
			if (task_result_queue->fx_param.fx_data_type == fx_data_type_double) {
				fprintf_s(task_result_queue->fileptrcsv, "lower%d;", i);
				fprintf_s(task_result_queue->fileptrcsv, "upper%d;", i);
			}
			fprintf_s(task_result_queue->fileptrcsv, "gene%d;", i);
		}
		
		
		if (task_result_queue->runtime_param.logging_param.include_config == 1) {
			for (int i = 0; i < task_result_queue->runtime_param.logging_param.config_int_count; i++)
			{
				fprintf_s(task_result_queue->fileptrcsv, "config_int%d;", i);
			}
			for (int i = 0; i < task_result_queue->runtime_param.logging_param.config_double_count; i++)
			{
				fprintf_s(task_result_queue->fileptrcsv, "config_double%d;", i);
			}
		}

		fprintf_s(task_result_queue->fileptrcsv, "\n");
	}
    // local variables for the file names
	free(filename_csv);
	free(filename_bin);
}

void close_file(task_result_queue_t* task_result_queue)
{
	if (task_result_queue->runtime_param.logging_param.write_bin == 1) {
		fclose(task_result_queue->fileptr);
	}

	if (task_result_queue->runtime_param.logging_param.write_csv == 1) {
		fclose(task_result_queue->fileptrcsv);
	}
}

void report_task(task_queue_t* task_queue, task_param_t* task, adaptive_memory_t* adaptive_memory, thread_param_t* thread_param, gene_pool_t* gene_pool, int best_result) {
	task_result_t task_result = {0};
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


	for (int position = 0; position < log_top_n; position++) {

        // invert the index to get best individuals (highest idx) first and lowest last
		int individual_id = gene_pool->sorted_indexes[gene_pool->individuals - position - 1];
		double result = gene_pool->pop_result_set[individual_id]; // result and invert if optim mode is minimisation
		
		copy_to_bin_buffer(&task_result, &adaptive_memory->iteration_counter, sizeof(int));
		copy_to_bin_buffer(&task_result, &task->task_id, sizeof(int));
		copy_to_bin_buffer(&task_result, &individual_id, sizeof(int));
		copy_to_bin_buffer(&task_result, &position, sizeof(int)); // position
		copy_to_bin_buffer(&task_result, &result, sizeof(double));
		copy_to_bin_buffer(&task_result, task->lower, sizeof(double) * thread_param->runtime_param.genes);
		copy_to_bin_buffer(&task_result, task->upper, sizeof(double) * thread_param->runtime_param.genes);
		copy_to_bin_buffer(&task_result, gene_pool->pop_param_double[individual_id], sizeof(double) * thread_param->runtime_param.genes);

		if (thread_param->runtime_param.logging_param.write_csv == 1) {
			task_result.csv_position += snprintf(
				task_result.csv_buffer + task_result.csv_position,
				(uint64_t) task_result.csv_buffer_length - task_result.csv_position,
				"%d;%d;%d;%d;%e;",
				adaptive_memory->iteration_counter,
				task->task_id,
				individual_id,
                position, // position
                result 
			);
			for (int i = 0; i < thread_param->runtime_param.genes; i++)
			{
				if (task->config_ga.fx_param.fx_data_type == fx_data_type_double) {
					task_result.csv_position += snprintf(
						task_result.csv_buffer + task_result.csv_position,
						(uint64_t)task_result.csv_buffer_length - task_result.csv_position,
						"%e;%e;%e;",
						task->lower[i],
						task->upper[i],
						gene_pool->pop_param_double[individual_id][i]
					);
				}
				else if (task->config_ga.fx_param.fx_data_type == fx_data_type_int) {
					task_result.csv_position += snprintf(
						task_result.csv_buffer + task_result.csv_position,
						(uint64_t)task_result.csv_buffer_length - task_result.csv_position,
						"%u;",
						gene_pool->pop_param_bin[individual_id][i]
					);

				}
			}
		}
		if (thread_param->runtime_param.logging_param.include_config == 1) {
			//task_result.config_int[0] = task->config_ga.mutation_param.mutation_rate;
			//task_result.config_double[0] = adaptive_memory->computed_mutation;
			//task_result.config_double[1] = adaptive_memory->convergence_moving_window;
			
			copy_to_bin_buffer(&task_result, &task->config_ga.mutation_param.mutation_rate, sizeof(double));
			copy_to_bin_buffer(&task_result, &adaptive_memory->computed_mutation, sizeof(double));
			copy_to_bin_buffer(&task_result, &adaptive_memory->convergence_moving_window, sizeof(double));

			if (thread_param->runtime_param.logging_param.write_csv == 1) {
				task_result.csv_position += snprintf(
					task_result.csv_buffer + task_result.csv_position,
					(uint64_t)task_result.csv_buffer_length - task_result.csv_position,
					"%e;%e;%e;",
                    task->config_ga.mutation_param.mutation_rate[individual_id],
					adaptive_memory->computed_mutation,
                    adaptive_memory->convergence_moving_window
				);
			}
		}
		if (thread_param->runtime_param.logging_param.write_csv == 1) {
			task_result.csv_position += snprintf(
				task_result.csv_buffer + task_result.csv_position,
				(uint64_t)task_result.csv_buffer_length - task_result.csv_position,
				"\n"
			);
		}
	}

	add_result(thread_param->task_queue->task_result_queue, &task_result);
}

void write_file_buffer(task_result_queue_t* task_result_queue, task_result_t* task_result) {
	if (task_result_queue->runtime_param.logging_param.write_bin == 1) {
		fwrite(task_result->bin_buffer, task_result->bin_position, 1, task_result_queue->fileptr);
	}
    if (task_result_queue->runtime_param.logging_param.write_csv == 1) {
        fwrite(task_result->csv_buffer, task_result->csv_position, 1, task_result_queue->fileptrcsv);
    }
}
