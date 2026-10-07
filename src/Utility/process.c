#include "process.h"
#include "fitness.h"

uint32_t dedupe_population(gene_pool_t* pool) {
    /* Preserve the adjacent equal-score scan and SIMD chromosome comparison.
       Mark duplicates separately: measured fitness and its ranking stay intact.
       This is deliberately not an exhaustive search within equal-score groups. */
    const uint32_t blocks = pool->individual_mem_size / sizeof(__m512i);
    uint32_t duplicates = 0;
    memset(pool->duplicate_flags, 0, (size_t)pool->individuals * sizeof(uint32_t));
    for (uint32_t rank = 0; rank + 1 < pool->individuals; ++rank) {
        const uint32_t a = pool->sorted_indexes[rank];
        const uint32_t b = pool->sorted_indexes[rank + 1];
        if (pool->pop_result_set[a] != pool->pop_result_set[b]) continue;
        const __m512i* left = (const __m512i*)pool->pop_param_bin[a];
        const __m512i* right = (const __m512i*)pool->pop_param_bin[b];
        int equal = 1;
        for (uint32_t j = 0; j < blocks; ++j) {
            /* Eight 64-bit lane comparisons produce an eight-bit mask. */
            if (_mm512_cmpeq_epu64_mask(left[j], right[j]) != UINT8_MAX) {
                equal = 0;
                break;
            }
        }
        const size_t compared = (size_t)blocks * sizeof(__m512i);
        if (equal && compared < pool->individual_mem_size)
            equal = memcmp((const char*)left + compared, (const char*)right + compared,
                           pool->individual_mem_size - compared) == 0;
        if (equal) {
            pool->duplicate_flags[a] = 1;
            ++duplicates;
        }
    }
    return duplicates;
}

static void reseed_population(gene_pool_t* pool, population_param_t* param, uint32_t duplicates) {
    /* Preserve the existing policy: replace the worst max(duplicates, bottom_N)
       slots, rather than changing it to replacement of each duplicate's slot.
       Protect elites even when the requested count exceeds the non-elite range. */
    uint32_t count = duplicates > param->reseed_bottom_N ? duplicates : param->reseed_bottom_N;
    const uint32_t available = pool->individuals - pool->elitism;
    if (count > available) count = available;
    pool->reseed_count = count;
    for (uint32_t rank = 0; rank < count; ++rank) {
        pool->reseed_indexes[rank] = pool->sorted_indexes[rank];
        fill_individual_uniform(pool, pool->reseed_indexes[rank]);
    }
}

void process_pop(gene_pool_t* pool, task_param_t* task, fx_task_queue_t* fx_task_queue) {
    process_fx(pool, task, fx_task_queue);
    memcpy(pool->sorted_indexes, pop_index_sequence, (size_t)pool->individuals * sizeof(uint32_t));
    indexed_bitonic_sort_8v(pool->pop_result_set, pool->sorted_indexes,
                           pool->sorted_indexes_temp, pool->individuals);
    const uint32_t duplicates = dedupe_population(pool);
    /* Dedupe no longer alters scores, so no second sort is necessary. */
    process_normalize(pool);
    process_flatten(pool, &task->config_ga.flatten_param);
    memcpy(pool->selected_indexes, pool->sorted_indexes, (size_t)pool->individuals * sizeof(uint32_t));
    process_selection(pool, &task->config_ga.selection_param);
    process_crossover(pool, &task->config_ga.crossover_param);
    process_mutation(pool, &task->config_ga.mutation_param);
    reseed_population(pool, &task->config_ga.population_param, duplicates);
}
