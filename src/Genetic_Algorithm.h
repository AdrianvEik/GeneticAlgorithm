#ifndef _GENETIC_ALGORITHM_H_
#define _GENETIC_ALGORITHM_H_

#define DllExport   __declspec( dllexport )

//DllExport double Genetic_Algorithm(task_queue_t* task_queue, config_ga_t config_ga, runtime_param_t runtime_param);

#include "Helper/Struct.h"
#include "Multiprocessing/mp_logger.h"

/**
 * Run the configured genetic algorithm and return the best objective value.
 *
 * The function validates inputs, optionally writes the JSON configuration,
 * creates the console, task, fitness, and result queues, starts solver and
 * logging threads, feeds generated tasks into the solver queue, waits for all
 * work to finish, and copies final progress into ``end_result`` when provided.
 *
 * :param config_ga: Genetic algorithm operator and objective configuration.
 * :param runtime_param: Runtime dimensions, task splitting, threading, random
 *     seed, and logging settings.
 * :param end_result: Optional progress snapshot filled with the final run
 *     statistics. Pass ``NULL`` when only the best result is needed.
 * :returns: Best weighted objective value observed across all completed tasks.
 */
double Genetic_Algorithm(config_ga_t config_ga, runtime_param_t runtime_param, progress_t* end_result);

/**
 * Re-export of :c:func:`default_runtime_param` for consumers including only
 * this public entry header.
 */
runtime_param_t default_runtime_param();

/**
 * Re-export of :c:func:`default_config` for consumers including only this
 * public entry header.
 */
config_ga_t default_config(runtime_param_t runtime_param);

//int main();

#endif // _GENETIC_ALGORITHM_H_
