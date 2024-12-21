#ifndef _GENETIC_ALGORITHM_H_
#define _GENETIC_ALGORITHM_H_

#define DllExport   __declspec( dllexport )

//DllExport double Genetic_Algorithm(task_queue_t* task_queue, config_ga_t config_ga, runtime_param_t runtime_param);

#include "Helper/Struct.h"

double Genetic_Algorithm(config_ga_t config_ga, runtime_param_t runtime_param);

runtime_param_t default_runtime_param();
config_ga_t default_config(runtime_param_t runtime_param);

//int main();

#endif // _GENETIC_ALGORITHM_H_