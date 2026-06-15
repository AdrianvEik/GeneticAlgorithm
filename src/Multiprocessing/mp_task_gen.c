
#include "mp_task_gen.h"



uint32_t compute_task_count(runtime_param_t* runtime_param) {
    uint32_t generated_task_count = 1;
    uint32_t tasks_per_gene = 1;
    uint32_t remaining_tasks = runtime_param->task_count_solver;

    for (uint32_t i = 0; i < runtime_param->genes; i++) {
		if (remaining_tasks == 1) {
			tasks_per_gene = 1;
		}
		else {
			if (remaining_tasks <= 2 * (runtime_param->genes - i)) {
				tasks_per_gene = 2;
			}
			else {
				tasks_per_gene = (uint32_t)nearbyint(pow(remaining_tasks, 1.0 / (runtime_param->genes - i)));

			}
		}
		remaining_tasks /= tasks_per_gene;
		generated_task_count *= tasks_per_gene;
	}
    return generated_task_count;
}

//void generate_task(task_param_t* task_list, int* task_id, runtime_param_t runtime_param, config_ga_t config_ga, int current_gene, int* tasks_per_gene, int* position) {
static void generate_task_per_gene(task_queue_t* task_queue, runtime_param_t runtime_param, config_ga_t config_ga, uint32_t current_gene, uint32_t* tasks_per_gene, int* position) {
	uint32_t bit_position = 0;

	for (uint32_t i = 0; i < tasks_per_gene[current_gene]; i++) {
		position[current_gene] = i;

		if (current_gene != runtime_param.genes - 1) {
			generate_task_per_gene(task_queue, runtime_param, config_ga, current_gene + 1, tasks_per_gene, position);
		}
		else {
			task_param_t task;
			init_task(runtime_param, config_ga, &task);
			if (config_ga.fx_param.fx_data_type == fx_data_type_int) {
				for (uint32_t j = 0; j < runtime_param.genes; j++) {
                    // mask over the last N-bits where N corresponds to the tasks per gene
					_BitScanReverse(&bit_position, tasks_per_gene[j]);
					task.zone_id[j] = position[j] << (32 - bit_position);
					task.zone_mask[j] = UINT32_MAX >> bit_position;
				}
			}
			else {
				for (uint32_t j = 0; j < runtime_param.genes; j++) {
					task.lower[j] = config_ga.population_param.lower[j] + (config_ga.population_param.upper[j] - config_ga.population_param.lower[j]) / tasks_per_gene[j] * (position[j]);
					task.upper[j] = config_ga.population_param.upper[j] - (config_ga.population_param.upper[j] - config_ga.population_param.lower[j]) / tasks_per_gene[j] * (tasks_per_gene[j] - position[j] - 1);
				}
			}
			add_task(task_queue, &task);
		}
	}
}

void make_task_list(runtime_param_t* runtime_param, config_ga_t config_ga, task_queue_t* task_queue) {
	//int task_id = 0;
	uint32_t task_count = runtime_param->task_count_solver; // power of 2

	if (runtime_param->zone_enable) {
		uint32_t remaining_tasks = task_count;

		//int generated_task_count = 1;
		uint32_t minimum_tasks_per_gene = 2;

		uint32_t* tasks_per_gene = (uint32_t*)malloc(sizeof(uint32_t) * runtime_param->genes);
		if (tasks_per_gene == NULL) EXIT_MEM_ERROR();

		uint32_t* position = (uint32_t*)malloc(sizeof(uint32_t) * runtime_param->genes);
		if (position == NULL) EXIT_MEM_ERROR();

		if (config_ga.fx_param.fx_data_type == fx_data_type_int) {
			minimum_tasks_per_gene = 1 << ((uint32_t)ceilf(log2f((float)minimum_tasks_per_gene)) - 1);
        }

		for (uint32_t i = 0; i < runtime_param->genes; i++) {
			if (remaining_tasks == 1) {
				tasks_per_gene[i] = 1;
			}
			else {
				if (remaining_tasks <= minimum_tasks_per_gene * (runtime_param->genes - i)) {
					tasks_per_gene[i] = minimum_tasks_per_gene;
				}
				else {
					if (config_ga.fx_param.fx_data_type == fx_data_type_int) {
						//float result = (((log2f(powf((float)remaining_tasks, 1.0f / (float)(runtime_param->genes - i))))));
						//printf("%f, %u", result, ((uint32_t)ceilf(log2f(powf((float)remaining_tasks, 1.0f / (float)(runtime_param->genes - i))))));
						tasks_per_gene[i] = (1u << ((uint32_t)ceilf(log2f(powf((float)remaining_tasks, 1.0f / (float)(runtime_param->genes - i))))));
					}
					else {
						tasks_per_gene[i] = (uint32_t)nearbyint(pow(remaining_tasks, 1.0f / (runtime_param->genes - i)));
					}
				}
			}
			remaining_tasks /= tasks_per_gene[i];
			//generated_task_count *= tasks_per_gene[i];
		}

		generate_task_per_gene(task_queue, *runtime_param, config_ga, 0u, tasks_per_gene, position);

		free(position);
		free(tasks_per_gene);
	}
    else {
        for (uint32_t i = 0; i < task_count; i++) {
            task_param_t task;
            init_task(*runtime_param, config_ga, &task);
			for (uint32_t j = 0; j < runtime_param->genes; j++) {
				task.lower[j] = config_ga.population_param.lower[j];
				task.upper[j] = config_ga.population_param.upper[j];
			}
            add_task(task_queue, &task);
        }
    }
}
