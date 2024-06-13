
#ifndef _GENETIC_ALGORITHM_H_
#define _GENETIC_ALGORITHM_H_

#include "Helper/Struct.h"
#define DllExport   __declspec( dllexport )

DllExport void Genetic_Algorithm(config_ga_t config_ga, runtime_param_t runtime_param);
DllExport int main();

#endif // _GENETIC_ALGORITHM_H_