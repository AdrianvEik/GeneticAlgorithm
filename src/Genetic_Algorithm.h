#include "Utility/process.h"
#include "Utility/pop.h"
#include "Utility/crossover.h"
#include "Utility/mutation.h"
#include "Utility/selection.h"
#include "Utility/flatten.h"

#include "Function/Function.h"

#include "Helper/Helper.h"
#include "Helper/Struct.h"
#include "Helper/multiprocessing.h"
#include "Helper/rng.h"

#include "Logger/logging.h"

#include "Optimisation/Optimizer.h"

#ifndef _GENETIC_ALGORITHM_H_
#define _GENETIC_ALGORITHM_H_

#include "Helper/Struct.h"
#define DllExport   __declspec( dllexport )

//DllExport double Genetic_Algorithm(task_queue_t* task_queue, config_ga_t config_ga, runtime_param_t runtime_param);

double Genetic_Algorithm(config_ga_t config_ga, runtime_param_t runtime_param);

runtime_param_t default_runtime_param();
config_ga_t default_config(runtime_param_t runtime_param);

//int main();

#endif // _GENETIC_ALGORITHM_H_