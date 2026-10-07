#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/Function/Function.h"

int main(void) {
    const int methods[] = {
        fx_method_Ackley, fx_method_Griewank, fx_method_Langermann,
        fx_method_Levy, fx_method_Rastrigin, fx_method_Schwefel
    };
    double (*functions[])(double*, uint32_t) = {
        Ackley_fx, Griewank_fx, Langermann_fx,
        Levy_fx, Rastrigin_fx, Schwefel_fx
    };
    uint32_t chromosomes[2][2] = {{0, UINT32_MAX}, {UINT32_MAX, 0}};
    uint32_t* binary[] = {chromosomes[0], chromosomes[1]};
    double decoded[2][2] = {{0}};
    double* parameters[] = {decoded[0], decoded[1]};
    double results[] = {123.0, 123.0};
    uint32_t sorted[] = {1, 0};
    _Atomic uint32_t ready[2];
    atomic_init(&ready[0], 0);
    atomic_init(&ready[1], 0);
    double lower[] = {1.0, 2.0};
    double upper[] = {3.0, 4.0};
    gene_pool_t pool = {0};
    task_param_t task = {0};
    pool.genes = 2;
    pool.individuals = 2;
    pool.pop_param_bin = binary;
    pool.pop_param_double = parameters;
    pool.pop_result_set = results;
    pool.sorted_indexes = sorted;
    pool.fx_ready = ready;
    task.lower = lower;
    task.upper = upper;
    task.config_ga.fx_param.fx_data_type = fx_data_type_double;
    double expected_point[] = {3.0, 2.0};

    for (unsigned i = 0; i < sizeof(methods) / sizeof(methods[0]); i++) {
        task.config_ga.fx_param.fx_method = methods[i];
        task.config_ga.fx_param.fx_optim_mode = fx_optim_mode_maximize;
        ready[0] = 0;
        process_fx_set(&pool, &task, 0, 0);
        double expected = -functions[i](expected_point, 2);
        if (!isfinite(results[1]) || fabs(results[1] - expected) > 1e-12 ||
            results[0] != 123.0 || ready[0] != 1 || ready[1] != 0 ||
            decoded[1][0] != 3.0 || decoded[1][1] != 2.0 ||
            task.config_ga.fx_param.fx_optim_mode != fx_optim_mode_minimize) {
            fprintf(stderr, "Benchmark dispatch failed for method %d\n", methods[i]);
            return EXIT_FAILURE;
        }
    }
    puts("Benchmark dispatch tests passed");
    return EXIT_SUCCESS;
}
