
#ifndef  _DUMP_CONFIG_H
#define _DUMP_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"
#include "../Multiprocessing/mp_solver_th.h"

void write_config(runtime_param_t runtime_param, config_ga_t config_ga);

#endif // ! _DUMP_CONFIG_H
