
#ifndef LOGGING_H
#define LOGGING_H 

#include "../helper/struct.h"

void write_param(gene_pool_t gene_pool, int iteration);
void write_config(gene_pool_t gene_pool, runtime_param_t run_param, config_ga_t config_ga);
void open_file(gene_pool_t gene_pool, runtime_param_t runtime_param);
void close_file();

#endif // LOGGING_H