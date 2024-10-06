
#ifndef FUNCTION_H
#define FUNCTION_H

#include "../Helper/Struct.h"

static const int fx_method_pointer = -1;
static const int fx_method_Styblinski_Tang = 0;
static const int fx_method_Wheelers_Ridge = 1;

void process_fx(gene_pool_t* gene_pool, fx_param_t* fx_param, double* lower, double* upper);
#endif
