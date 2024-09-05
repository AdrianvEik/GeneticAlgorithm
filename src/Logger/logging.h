
#ifndef LOGGING_H
#define LOGGING_H 

#include "../helper/struct.h"

void write_param(gene_pool_t gene_pool, thread_param_t thread_param, int iteration);
void write_config(gene_pool_t gene_pool, thread_param_t thread_param);
void open_file(gene_pool_t gene_pool, thread_param_t* thread_param, char* log_file);
void close_file(thread_param_t thread_param);

#endif // LOGGING_H