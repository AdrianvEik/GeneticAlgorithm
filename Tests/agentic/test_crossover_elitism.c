/* Exercise the real crossover dispatcher, elite copying, and buffer swap. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/Utility/pop.h"
#include "../../src/Utility/crossover.h"
#include "../../src/Helper/rng.h"

static int check_elites(uint32_t elites, int permuted) {
    runtime_param_t runtime = default_runtime_param();
    runtime.individuals = 64;
    runtime.genes = 16;
    runtime.elitism = elites;
    gene_pool_t pool = {0};
    init_gene_pool(&pool, &runtime);
    unsigned char* expected = malloc(pool.individuals * pool.individual_mem_size);
    if (!expected) {
        free_gene_pool(&pool);
        return 1;
    }
    seed_rand_task_threadlocal(12345, 0);
    crossover_param_t crossover = {0};
    crossover.crossover_method = crossover_method_complete;
    int failed = 0;
    /* Repeat with the two allocations exchanging roles each time. */
    for (uint32_t generation = 0; generation < 3 && !failed; ++generation) {
        for (uint32_t rank = 0; rank < pool.individuals; ++rank) {
            pool.sorted_indexes[rank] = permuted ? (17 * rank + 7) % 64 : rank;
            pool.selected_indexes[rank] = 5; /* Identical parents: exact child oracle. */
            for (uint32_t gene = 0; gene < pool.individual_mem_size / sizeof(uint32_t); ++gene)
                pool.pop_param_bin[rank][gene] = ((generation + 1) << 24) | (rank << 8) | gene;
            memset(pool.pop_param_bin_cross_buffer[rank], 0xA5, pool.individual_mem_size);
        }
        for (uint32_t rank = 0; rank < pool.individuals; ++rank) {
            const uint32_t slot = pool.sorted_indexes[rank];
            const uint32_t source = rank >= pool.individuals - elites ? slot : 5;
            memcpy(expected + slot * pool.individual_mem_size,
                   pool.pop_param_bin[source], pool.individual_mem_size);
        }
        uint32_t** previous_active = pool.pop_param_bin;
        uint32_t** previous_alternate = pool.pop_param_bin_cross_buffer;
        process_crossover(&pool, &crossover);
        if (pool.pop_param_bin != previous_alternate ||
            pool.pop_param_bin_cross_buffer != previous_active) {
            fprintf(stderr, "Crossover did not swap buffers\n");
            failed = 1;
        }
        for (uint32_t rank = 0; rank < pool.individuals; ++rank) {
            const uint32_t slot = pool.sorted_indexes[rank];
            if (memcmp(pool.pop_param_bin[slot], expected + slot * pool.individual_mem_size,
                       pool.individual_mem_size) != 0) {
                fprintf(stderr, "Wrong chromosome: elites=%u permuted=%d generation=%u rank=%u slot=%u\n",
                        elites, permuted, generation, rank, slot);
                failed = 1;
                break;
            }
        }
    }
    free(expected);
    free_gene_pool(&pool);
    return failed;
}

int main(void) {
    const uint32_t elite_counts[] = {2, 1, 3, 0, 64};
    for (unsigned i = 0; i < sizeof(elite_counts) / sizeof(elite_counts[0]); ++i)
        for (int permuted = 0; permuted <= 1; ++permuted)
            if (check_elites(elite_counts[i], permuted)) return EXIT_FAILURE;
    puts("Crossover elite preservation tests passed");
    return EXIT_SUCCESS;
}
