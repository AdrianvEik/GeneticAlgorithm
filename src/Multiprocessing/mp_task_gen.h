
#ifndef _MP_TASK_GEN_H_
#define _MP_TASK_GEN_H_

#include "../Helper/Struct.h"
#include "mp_solver_th.h"

//// Forward declarations of structs from other headers
//struct runtime_param_t;  // From "../Helper/Struct.h"
//struct config_ga_t;       // From "../Helper/Struct.h"
//struct task_queue_t; // From "mp_solver_th.h"

int compute_task_count(runtime_param_t runtime_param);
void make_task_list(runtime_param_t* runtime_param, config_ga_t config_ga, task_queue_t* task_queue);

#endif _MP_TASK_GEN_H_
