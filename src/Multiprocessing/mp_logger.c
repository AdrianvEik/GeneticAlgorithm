
#include "mp_logger.h"


void init_task_result_queue(task_result_queue_t* task_result_queue, runtime_param_t runtime_param, console_queue_t* console_queue, fx_param_t fx_param) {
	task_result_queue->result_list = (task_result_t*)malloc(sizeof(task_result_t) * runtime_param.logging_param.queue_size);
	if (task_result_queue->result_list == NULL) EXIT_MEM_ERROR();

	task_result_queue->lock = (thread_mutex_t*)malloc(sizeof(thread_mutex_t));
	if (task_result_queue->lock == NULL) EXIT_MEM_ERROR();

	task_result_queue->progress = (progress_t){0};
	task_result_queue->progress.best_result = NAN;
	task_result_queue->progress.best_result_iteration = UINT32_MAX;
	task_result_queue->progress.average_result = NAN;
	task_result_queue->progress.result_standard_deviation = NAN;
	task_result_queue->progress.max_tasks = runtime_param.task_count_solver;
	task_result_queue->result_m2 = 0;

	task_result_queue->fx_param = fx_param;

    task_result_queue->console_queue = console_queue;

	task_result_queue->first_task_id = 0;
	task_result_queue->next_task_id = 0;
	task_result_queue->runtime_param = runtime_param;

	task_result_queue->bin_single_entry_length = sizeof(int) * 4 + sizeof(double) + sizeof(double) * runtime_param.genes * 3;
	if (runtime_param.logging_param.include_config == 1) {
		task_result_queue->bin_single_entry_length += sizeof(int) * runtime_param.logging_param.config_int_count + sizeof(double) * runtime_param.logging_param.config_double_count;
	}


	if (runtime_param.logging_param.write_csv == 1) {
		const uint32_t len_of_engineering_double = 15; // -1.123456e+123;
		const uint32_t len_of_formatted_int = 12; // -12345678901;
		if (fx_param.fx_data_type == fx_data_type_double) {
			task_result_queue->csv_single_entry_length = 4 * len_of_formatted_int + 1 * len_of_engineering_double + runtime_param.genes * len_of_engineering_double * 3;
		}
		else if(fx_param.fx_data_type == fx_data_type_int) {
			task_result_queue->csv_single_entry_length = 4 * len_of_formatted_int + 1 * len_of_formatted_int + runtime_param.genes * len_of_formatted_int;
		}
		else {
			EXIT_WITH_ERROR("Unknown fx_data_type in init_task_result_queue\n", 1);
        }
		if (runtime_param.logging_param.include_config == 1) {
			task_result_queue->csv_single_entry_length += runtime_param.logging_param.config_int_count * len_of_formatted_int + runtime_param.logging_param.config_double_count * len_of_engineering_double;
		}

	}
	thread_mutex_init(task_result_queue->lock);
}

void free_task_result_queue(task_result_queue_t* task_result_queue) {
	thread_mutex_destroy(task_result_queue->lock);
    free(task_result_queue->result_list);
    free(task_result_queue->lock);
}

int record_completed_result(task_result_queue_t* queue, const task_result_t* result) {
    if (result->task_type != BEST_RESULT_TASK) return 0;
    progress_t* progress = &queue->progress;
    const uint32_t count = ++progress->tasks_completed;
    const double value = result->result;
    if (count == 1) {
        progress->average_result = value;
        progress->optim_mode = result->optim_mode;
        queue->result_m2 = 0;
    } else {
        const double delta = value - progress->average_result;
        progress->average_result += delta / count;
        queue->result_m2 += delta * (value - progress->average_result);
    }
    progress->result_standard_deviation = count > 1
        ? sqrt(queue->result_m2 / (count - 1)) : NAN;

    if (count == 1 || (result->optim_mode == fx_optim_mode_minimize
        ? value < progress->best_result : value > progress->best_result)) {
        progress->best_result = value;
        progress->best_result_iteration = result->iteration;
        return 1;
    }
    return 0;
}

void stop_result_logger(task_result_queue_t* task_result_queue, uint32_t thread_count, double* best_res) {
	for (uint32_t i = 0; i < thread_count; i++) {
		task_result_t result;
		result.task_type = TERMINATE_THREAD;
		add_result(task_result_queue, &result);
	}

	thread_join(task_result_queue->thread_id);

	*best_res = task_result_queue->progress.best_result;
}


void init_task_result(task_result_queue_t* task_result_queue, task_result_t* task_result, uint32_t entry_count) {
	if (entry_count == 0) {
		return;
	}

	task_result->bin_buffer = malloc(sizeof(unsigned char) * task_result_queue->bin_single_entry_length * entry_count);
	if (task_result->bin_buffer == NULL) EXIT_MEM_ERROR();

	if (task_result_queue->runtime_param.logging_param.write_csv == 1) {
		task_result->csv_buffer = malloc(sizeof(char) * task_result_queue->csv_single_entry_length * entry_count);
		if (task_result->csv_buffer == NULL) EXIT_MEM_ERROR();
		task_result->csv_buffer[0] = '\0';
	}
	else {
		task_result->csv_buffer = NULL;
	}

	task_result->bin_position = 0;
	task_result->csv_position = 0;
	//DEBUG
	task_result->bin_buffer_length = task_result_queue->bin_single_entry_length * entry_count;
	task_result->csv_buffer_length = task_result_queue->csv_single_entry_length * entry_count;
}

void add_result(task_result_queue_t* task_result_queue, task_result_t* result) {
	int result_added = 0;
	while (!result_added) {
		thread_mutex_lock(task_result_queue->lock);
		if (task_result_queue->first_task_id == (task_result_queue->next_task_id + 1) % task_result_queue->runtime_param.logging_param.queue_size) {
			thread_mutex_unlock(task_result_queue->lock);
			Sleep(1000);
			continue;
		}
		task_result_queue->result_list[task_result_queue->next_task_id] = *result;
		task_result_queue->next_task_id = (task_result_queue->next_task_id + 1) % task_result_queue->runtime_param.logging_param.queue_size;
		result_added = 1;
		thread_mutex_unlock(task_result_queue->lock);
	}
}

int get_result(task_result_queue_t* task_result_queue, task_result_t* result) {
	thread_mutex_lock(task_result_queue->lock);
	if (task_result_queue->first_task_id == task_result_queue->next_task_id) {
		thread_mutex_unlock(task_result_queue->lock);
		return 0;
	}
    *result = task_result_queue->result_list[task_result_queue->first_task_id];
	task_result_queue->first_task_id = (task_result_queue->first_task_id + 1) % task_result_queue->runtime_param.logging_param.queue_size;
	thread_mutex_unlock(task_result_queue->lock);
	return 1;
}

void free_task_result(task_result_t* task_result) {
	if (task_result->csv_buffer != NULL) {
		free(task_result->csv_buffer);
	}
	free(task_result->bin_buffer);
}
