
#ifndef PROCESS_H
#define PROCESS_H

typedef struct gene_pool_s gene_pool_t;
typedef struct task_param_s task_param_t;

// gen purpose
void process_pop(gene_pool_t* gene_pool, task_param_t* task);

void init_pre_compute(gene_pool_t* gene_pool);
void free_pre_compute();

#endif