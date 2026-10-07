#ifndef GA_FITNESS_H
#define GA_FITNESS_H

#include "../Helper/Struct.h"

/**
 * Copy canonical fitness in physical order, then normalize the copy in place.
 * Requires finite scores and sorted_indexes in ascending canonical order.
 * Exact -DBL_MAX/+DBL_MAX map to 0/1 and do not determine the interior range.
 * Other scores use (f-min)/(max-min); an equal interior range maps to 1.
 * Opposite-sign ranges that would overflow use half-sized differences.
 * Equal scores remain possible: dedupe tests genes too and reseeding is later.
 * Genes, canonical scores and sorted indexes are not modified.
 */
void process_normalize(gene_pool_t* gene_pool);

/** Stable logistic function, with no positive exponential arguments. */
double fitness_sigmoid(double value);

/** Logistic of difference / temperature, avoiding division overflow.
 * Preconditions: finite difference in [-1,1], positive finite temperature.
 */
double fitness_acceptance(double difference, double temperature);

#endif
