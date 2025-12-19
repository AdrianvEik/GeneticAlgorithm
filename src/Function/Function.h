
#ifndef FUNCTION_H
#define FUNCTION_H

#include <math.h>
#include <stdio.h>
#include <windows.h>

#include "../Helper/Helper.h"
#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"

#include "../Multiprocessing/mp_thread_locals.h"
#include "../Multiprocessing/mp_solver_th.h"

void process_fx_set(gene_pool_t* gene_pool, task_param_t* task, uint32_t individual_min, uint32_t individual_max);

void process_fx(gene_pool_t* gene_pool, task_param_t* task, fx_task_queue_t* fx_task_queue);
#endif
