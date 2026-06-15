
#ifndef _MP_TASK_GEN_H_
#define _MP_TASK_GEN_H_

#include <stdlib.h>
#include <math.h>

#include "mp_solver_th.h"
#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"
#include "../Function/Function.h"


//// Forward declarations of structs from other headers
//struct runtime_param_t;  // From "../Helper/Struct.h"
//struct config_ga_t;       // From "../Helper/Struct.h"
//struct task_queue_t; // From "mp_solver_th.h"

/**
 * Estimate how many solver tasks will be generated for zoned execution.
 *
 * :param runtime_param: Runtime settings containing gene and requested task
 *     counts.
 * :returns: Product of the per-gene task split selected by the generator.
 */
uint32_t compute_task_count(runtime_param_t* runtime_param);

/**
 * Generate solver tasks and enqueue them into the task queue.
 *
 * With zone splitting enabled, the search bounds or integer masks are divided
 * across genes before tasks are queued. With zone splitting disabled, the
 * configured global bounds are copied into each task.
 *
 * :param runtime_param: Runtime dimensions and task-count settings.
 * :param config_ga: Configuration whose population bounds and objective type
 *     determine task zones.
 * :param task_queue: Destination solver task queue.
 */
void make_task_list(runtime_param_t* runtime_param, config_ga_t config_ga, task_queue_t* task_queue);

#endif // _MP_TASK_GEN_H_
