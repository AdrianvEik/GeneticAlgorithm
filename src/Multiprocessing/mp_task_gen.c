
#include "mp_task_gen.h"

#include <stdlib.h>
#include <math.h>
#include "mp_solver_th.h"
#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"

int compute_task_count(runtime_param_t* runtime_param) {
    int task_count = 1;
    int generated_task_count = 1;
    int tasks_per_gene = 1;
    int remaining_tasks = runtime_param->task_count;

    for (int i = 0; i < runtime_param->genes; i++) {
		if (remaining_tasks == 1) {
			tasks_per_gene = 1;
		}
		else {
			if (remaining_tasks <= 2 * (runtime_param->genes - i)) {
				tasks_per_gene = 2;
			}
			else {
				tasks_per_gene = (int)nearbyint(pow(remaining_tasks, 1.0 / (runtime_param->genes - i)));

			}
		}
		remaining_tasks /= tasks_per_gene;
		generated_task_count *= tasks_per_gene;
	}
    return generated_task_count;
}

//void generate_task(task_param_t* task_list, int* task_id, runtime_param_t runtime_param, config_ga_t config_ga, int current_gene, int* tasks_per_gene, int* position) {
void generate_task_per_gene(task_queue_t* task_queue, runtime_param_t runtime_param, config_ga_t config_ga, int current_gene, int* tasks_per_gene, int* position) {

	for (int i = 0; i < tasks_per_gene[current_gene]; i++) {
		position[current_gene] = i;

		if (current_gene != runtime_param.genes - 1) {
			generate_task_per_gene(task_queue, runtime_param, config_ga, current_gene + 1, tasks_per_gene, position);
		}
		else {
			task_param_t task;
			init_task(runtime_param, config_ga, &task);
			for (int j = 0; j < runtime_param.genes; j++) {
				task.lower[j] = config_ga.population_param.lower[j] + (config_ga.population_param.upper[j] - config_ga.population_param.lower[j]) / tasks_per_gene[j] * (position[j]);
				task.upper[j] = config_ga.population_param.upper[j] - (config_ga.population_param.upper[j] - config_ga.population_param.lower[j]) / tasks_per_gene[j] * (tasks_per_gene[j] - position[j] - 1);
			}

			add_task(task_queue, &task);
		}
	}

}

void make_task_list(runtime_param_t* runtime_param, config_ga_t config_ga, task_queue_t* task_queue) {
	int task_id = 0;
	int task_count = runtime_param->task_count; // power of 2

	if (runtime_param->zone_enable == 1) {
		int remaining_tasks = task_count;

		int generated_task_count = 1;
		int minimum_tasks_per_gene = 2;

		int* tasks_per_gene = (int*)malloc(sizeof(int) * runtime_param->genes);
		if (tasks_per_gene == NULL) EXIT_MEM_ERROR();

		int* position = (int*)malloc(sizeof(int) * runtime_param->genes);
		if (position == NULL) EXIT_MEM_ERROR();

		for (int i = 0; i < runtime_param->genes; i++) {
			if (remaining_tasks == 1) {
				tasks_per_gene[i] = 1;
			}
			else {
				if (remaining_tasks <= minimum_tasks_per_gene * (runtime_param->genes - i)) {
					tasks_per_gene[i] = minimum_tasks_per_gene;
				}
				else {
					tasks_per_gene[i] = (int)nearbyint(pow(remaining_tasks, 1.0 / (runtime_param->genes - i)));

				}
			}
			remaining_tasks /= tasks_per_gene[i];
			generated_task_count *= tasks_per_gene[i];
		}

		generate_task_per_gene(task_queue, *runtime_param, config_ga, 0, tasks_per_gene, position);

		free(position);
		free(tasks_per_gene);
	}
    else {
        for (int i = 0; i < task_count; i++) {
            task_param_t task;
            init_task(*runtime_param, config_ga, &task);
			for (int j = 0; j < runtime_param->genes; j++) {
				task.lower[j] = config_ga.population_param.lower[j];
				task.upper[j] = config_ga.population_param.upper[j];
			}
            add_task(task_queue, &task);
        }
    }

}
