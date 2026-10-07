/* Result contracts: aggregation, objective units, serialization, and API. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../../src/Genetic_Algorithm.h"
#include "../../src/Logger/logging.h"

static void check_statistics(int direction, double offset, int reverse) {
    const double values[] = {2, 4, 4, 4, 5, 5, 7, 9};
    runtime_param_t runtime = default_runtime_param();
    runtime.logging_param.write_csv = 0;
    fx_param_t fx = {0};
    task_result_queue_t queue = {0};
    init_task_result_queue(&queue, runtime, NULL, fx);
    assert(isnan(queue.progress.best_result));
    assert(isnan(queue.progress.average_result));
    assert(isnan(queue.progress.result_standard_deviation));
    assert(queue.progress.best_result_iteration == UINT32_MAX);
    task_result_t result = {0};
    result.task_type = LOG_TASK;
    assert(!record_completed_result(&queue, &result));
    assert(queue.progress.tasks_completed == 0);
    result.task_type = BEST_RESULT_TASK;
    result.optim_mode = direction;
    for (unsigned i = 0; i < 8; ++i) {
        unsigned index = reverse ? 7 - i : i;
        result.result = offset + values[index];
        result.iteration = index + 10;
        record_completed_result(&queue, &result);
        if (i == 0) {
            assert(queue.progress.average_result == result.result);
            assert(isnan(queue.progress.result_standard_deviation));
        }
    }
    assert(queue.progress.tasks_completed == 8);
    assert(fabs(queue.progress.average_result - (offset + 5)) < 0.001);
    assert(fabs(queue.progress.result_standard_deviation - sqrt(32.0 / 7)) < 0.0001);
    assert(queue.progress.best_result == offset + (direction == -1 ? 2 : 9));
    assert(queue.progress.best_result_iteration == (direction == -1 ? 10 : 17));
    assert(queue.progress.optim_mode == direction);
    result.result = queue.progress.best_result;
    result.iteration = 999;
    assert(!record_completed_result(&queue, &result)); /* Ties keep first report. */
    free_task_result_queue(&queue);
}

static void check_reporting(int direction) {
    runtime_param_t runtime = default_runtime_param();
    runtime.genes = 1;
    runtime.logging_param.write_csv = 1;
    runtime.logging_param.include_config = 0;
    runtime.logging_param.top_n_export = 1;
    fx_param_t fx = {0};
    fx.fx_data_type = fx_data_type_double;
    fx.fx_optim_mode = 1; /* A built-in can override the original configuration. */
    task_result_queue_t queue = {0};
    init_task_result_queue(&queue, runtime, NULL, fx);
    task_queue_t tasks = {0};
    tasks.task_result_queue = &queue;
    thread_param_t thread = {0};
    thread.task_queue = &tasks;
    thread.runtime_param = runtime;
    double lower = -10, upper = 10, point = 2, score = 4;
    double* parameters[] = {&point};
    uint32_t sorted = 0;
    gene_pool_t pool = {0};
    pool.individuals = 1;
    pool.genes = 1;
    pool.iteration_number = 12;
    pool.sorted_indexes = &sorted;
    pool.pop_result_set = &score;
    pool.pop_param_double = parameters;
    task_param_t task = {0};
    task.lower = &lower;
    task.upper = &upper;
    task.config_ga.fx_param = fx;
    task.config_ga.fx_param.fx_optim_mode = direction;
    adaptive_memory_t adaptive = {0};
    for (int final = 0; final <= 1; ++final) {
        report_task(&tasks, &task, &adaptive, &thread, &pool, final);
        task_result_t result = {0};
        assert(get_result(&queue, &result));
        double binary_value;
        memcpy(&binary_value, result.bin_buffer + 4 * sizeof(int), sizeof(double));
        assert(binary_value == direction * 4);
        assert(strstr(result.csv_buffer, direction == -1 ? ";-4.000000e+00;" : ";4.000000e+00;") != NULL);
        assert(result.optim_mode == direction);
        assert(record_completed_result(&queue, &result) == final);
        if (final) {
            assert(result.result == direction * 4);
            assert(queue.progress.best_result == direction * 4);
            assert(queue.progress.best_result_iteration == 12);
        }
        free_task_result(&result);
    }
    assert(score == 4); /* Reporting never changes internal ranking scores. */
    free_task_result_queue(&queue);
}

static double constant_objective(void* parameters, uint32_t genes) {
    (void)parameters;
    (void)genes;
    return -7.5;
}

static void check_public_result(int direction) {
    runtime_param_t runtime = default_runtime_param();
    runtime.individuals = 64;
    runtime.genes = 16;
    runtime.elitism = 0;
    runtime.task_count_solver = 2;
    runtime.thread_count_solver = 1;
    runtime.random_seed = 12345;
    runtime.zone_enable = 0;
    runtime.logging_param.write_csv = 0;
    runtime.logging_param.write_bin = 0;
    runtime.logging_param.write_config = 0;
    runtime.logging_param.console_enabled = 0;
    runtime.logging_param.fully_qualified_basename = "result-contract-test";
    config_ga_t config = default_config(runtime);
    config.fx_param.fx_method = fx_method_pointer;
    config.fx_param.fx_data_type = fx_data_type_double;
    config.fx_param.fx_function = constant_objective;
    config.fx_param.fx_optim_mode = direction;
    config.optimizer_param.max_iterations = 0;
    for (uint32_t i = 0; i < runtime.genes; ++i) {
        config.population_param.lower[i] = -1;
        config.population_param.upper[i] = 1;
    }
    progress_t progress;
    double result = Genetic_Algorithm(config, runtime, &progress);
    assert(result == -7.5);
    assert(progress.best_result == result);
    assert(progress.average_result == result);
    assert(progress.result_standard_deviation == 0);
    assert(progress.tasks_completed == 2);
    assert(progress.optim_mode == direction);
    free_config_ga(&config);
}

int main(void) {
    for (int direction = -1; direction <= 1; direction += 2) {
        check_statistics(direction, 0, 0);
        check_statistics(direction, 0, 1);
        check_statistics(direction, 1e12, 0);
        check_statistics(direction, 1e12, 1);
        check_reporting(direction);
        check_public_result(direction);
    }
    puts("Result contract tests passed");
    return 0;
}
