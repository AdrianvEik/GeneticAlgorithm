
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Struct.h"

#include "multiprocessing.h"
void generate_task(task_param_t* task_list, int* task_id, runtime_param_t runtime_param, config_ga_t config_ga, int current_gene, int* tasks_per_gene, int* position) {

	for (int i = 0; i < tasks_per_gene[current_gene]; i++) {
		position[current_gene] = i;

		if (current_gene != runtime_param.genes - 1) {
			generate_task(task_list, task_id, runtime_param, config_ga, current_gene + 1, tasks_per_gene, position);
		}
		else {
			task_list[*task_id].task_id = *task_id;
			task_list[*task_id].status = 0;
			task_list[*task_id].lower = (double*)malloc(sizeof(double) * runtime_param.genes);
			task_list[*task_id].upper = (double*)malloc(sizeof(double) * runtime_param.genes);
			task_list[*task_id].paramset = (double*)malloc(sizeof(double) * runtime_param.genes);

			if (task_list[*task_id].lower == NULL || task_list[*task_id].upper == NULL || task_list[*task_id].paramset == NULL) {
				printf("Memory allocation failed");
				exit(255);
			}

			for (int j = 0; j < runtime_param.genes; j++) {
				task_list[*task_id].lower[j] = config_ga.fx_param.lower[j] + (config_ga.fx_param.upper[j] - config_ga.fx_param.lower[j]) / tasks_per_gene[j] * (position[j]);
				task_list[*task_id].upper[j] = config_ga.fx_param.upper[j] - (config_ga.fx_param.upper[j] - config_ga.fx_param.lower[j]) / tasks_per_gene[j] * (tasks_per_gene[j] - position[j] - 1);
			}

			(*task_id)++;
		}
	}

}

task_param_t* make_task_list(runtime_param_t* runtime_param, config_ga_t config_ga) {
	int task_id = 0;
	int task_count = runtime_param->task_count; // power of 2
	int remaining_tasks = task_count;

	int generated_task_count = 1;
	int minimum_tasks_per_gene = 2;

	int* tasks_per_gene = (int*)malloc(sizeof(int) * runtime_param->genes);

	if (tasks_per_gene == NULL) {
		printf("Memory allocation failed");
		exit(255);
	}

	for (int i = 0; i < runtime_param->genes; i++) {
		if (remaining_tasks == 1) {
			tasks_per_gene[i] = 1;
		}
		else {
			if (remaining_tasks <= minimum_tasks_per_gene * (runtime_param->genes - i)) {
				tasks_per_gene[i] = minimum_tasks_per_gene;
			}
			else {
				tasks_per_gene[i] = (int) pow(remaining_tasks, 1.0 / (runtime_param->genes - i));

			}
		}
		remaining_tasks /= tasks_per_gene[i];
		generated_task_count *= tasks_per_gene[i];

	}
	

	runtime_param->task_count = generated_task_count;

	int* position = (int*)malloc(sizeof(int) * runtime_param->genes);
	if (position == NULL) {
		printf("Memory allocation failed");
		exit(255);
	}

	task_param_t* task_list = (task_param_t*)malloc(sizeof(task_param_t) * generated_task_count);

	generate_task(task_list, &task_id, runtime_param[0], config_ga, 0, tasks_per_gene, position);

	free(position);
	free(tasks_per_gene);

	return task_list;
}



void free_task_list(task_param_t* task_list, runtime_param_t runtime_param) {
	for (int i = 0; i < runtime_param.task_count; i++) {
		free(task_list[i].lower);
		free(task_list[i].upper);
		free(task_list[i].paramset);
	}
	free(task_list);
}