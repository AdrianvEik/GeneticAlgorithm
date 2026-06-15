
#ifndef  _DUMP_CONFIG_H
#define _DUMP_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Helper/Struct.h"
#include "../Helper/error_handling.h"
#include "../Multiprocessing/mp_solver_th.h"

/**
 * Write the runtime and GA configuration to ``<basename>.json``.
 *
 * The JSON file mirrors the parameter groups stored in
 * :c:type:`runtime_param_t` and :c:type:`config_ga_t`.
 *
 * :param runtime_param: Runtime settings, including output basename.
 * :param config_ga: Operator and objective configuration to export.
 */
void write_config(runtime_param_t runtime_param, config_ga_t config_ga);

#endif // ! _DUMP_CONFIG_H
