#include "fitness.h"
#include <float.h>
#include <math.h>
#include <string.h>

void process_normalize(gene_pool_t* pool) {
    const uint32_t n = pool->individuals;
    if (n == 0) return;
    memcpy(pool->normalized_result_set, pool->pop_result_set, (size_t)n * sizeof(double));

    /* Trim reserved endpoints from the sorted view, not from physical storage. */
    uint32_t first = 0, end = n;
    while (first < end && pool->pop_result_set[pool->sorted_indexes[first]] == -DBL_MAX) ++first;
    while (end > first && pool->pop_result_set[pool->sorted_indexes[end - 1]] == DBL_MAX) --end;
    double low = 0.0, high = 0.0;
    if (first < end) {
        low = pool->pop_result_set[pool->sorted_indexes[first]];
        high = pool->pop_result_set[pool->sorted_indexes[end - 1]];
    }
    /* Test before subtracting: even finite extrema can have an infinite range. */
    const int half_range = low < 0.0 && high > 0.0 && high > DBL_MAX + low;
    const double range = half_range ? high * 0.5 - low * 0.5 : high - low;
    for (uint32_t rank = 0; rank < n; ++rank) {
        const uint32_t i = pool->sorted_indexes[rank];
        const double value = pool->normalized_result_set[i];
        double normalized;
        if (value == -DBL_MAX) normalized = 0.0;
        else if (value == DBL_MAX) normalized = 1.0;
        else if (high == low) normalized = 1.0;
        else normalized = half_range
            ? (value * 0.5 - low * 0.5) / range
            : (value - low) / range;
        /* Only contain endpoint rounding; this is not parameter normalization. */
        pool->normalized_result_set[i] = fmin(1.0, fmax(0.0, normalized));
    }
}

double fitness_sigmoid(double value) {
    if (value >= 0.0) return 1.0 / (1.0 + exp(-value));
    const double e = exp(value);
    return e / (1.0 + e);
}

double fitness_acceptance(double difference, double temperature) {
    if (difference == 0.0) return 0.5;
    /* Compare before dividing so subnormal positive temperatures are safe. */
    if (temperature <= fabs(difference) / 745.0) return difference > 0.0 ? 1.0 : 0.0;
    return fitness_sigmoid(difference / temperature);
}
