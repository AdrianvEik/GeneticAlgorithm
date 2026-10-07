/* One isolated, seeded run through the production Genetic_Algorithm entry point.
 * JSON goes to stdout only after success; the controller saves it atomically. */
#include "Genetic_Algorithm.h"
#include "Function/Benchmarks.h"
#include <errno.h>
#include <inttypes.h>

typedef struct {
    uint32_t generation;
    double seconds, objective;
} sample_t;

typedef struct {
    uint32_t generations, interval, completed, count, target_generation;
    double seconds, reference, tolerance, target_seconds;
    double solution[32];
    sample_t* samples;
} observation_t;

static void observe(const gene_pool_t* pool, uint32_t task_id, double seconds, void* context) {
    (void)task_id;
    observation_t* o = context;
    const uint32_t best = pool->sorted_indexes[pool->individuals - 1];
    const double objective = -pool->pop_result_set[best];
    o->seconds += seconds;
    o->completed++;
    if (!o->target_generation && objective - o->reference <= o->tolerance * pool->genes) {
        o->target_generation = o->completed;
        o->target_seconds = o->seconds;
    }
    if (o->completed == 1 || o->completed % o->interval == 0 || o->completed == o->generations) {
        sample_t* s = &o->samples[o->count++];
        s->generation = o->completed;
        s->seconds = o->seconds;
        s->objective = objective;
    }
    if (o->completed == o->generations)
        memcpy(o->solution, pool->pop_param_double[best], pool->genes * sizeof(double));
}

static uint32_t integer(const char* value, uint32_t low, uint32_t high) {
    char* end;
    errno = 0;
    unsigned long long n = strtoull(value, &end, 10);
    if (errno || !*value || *end || n < low || n > high) {
        fprintf(stderr, "Invalid integer: %s (expected %u..%u)\n", value, low, high);
        exit(2);
    }
    return (uint32_t)n;
}

int main(int argc, char** argv) {
    if (argc != 10) {
        fprintf(stderr, "Usage: ga_bench PROBLEM POPULATION GENERATIONS SEED MUTATION SELECTION FLATTEN CROSSOVER TRACE_INTERVAL\n"
            "PROBLEM: styblinski_tang, ackley, griewank, levy, rastrigin, schwefel; 32 genes.\n");
        return 2;
    }
    runtime_param_t r = default_runtime_param();
    r.genes = 32;
    r.individuals = integer(argv[2], 128, 512);
    if (r.individuals != 128 && r.individuals != 256 && r.individuals != 512) return 2;
    const uint32_t generations = integer(argv[3], 2, 100000000);
    r.random_seed = integer(argv[4], 1, UINT32_MAX);
    r.elitism = 3;
    r.task_count_solver = r.thread_count_solver = 1;
    r.task_size_fx = r.thread_count_fx = 0;
    r.zone_enable = 0;
    r.logging_param.write_csv = r.logging_param.write_bin = 0;
    r.logging_param.write_config = r.logging_param.console_enabled = 0;
    r.logging_param.export_interval = 0;
    r.logging_param.fully_qualified_basename = "ga_bench_unused";
    config_ga_t c = default_config(r);
    c.mutation_param.mutation_method = (int)integer(argv[5], 0, 1);
    c.selection_param.selection_method = (int)integer(argv[6], 0, 7);
    c.flatten_param.flatten_method = (int)integer(argv[7], 0, 5);
    c.crossover_param.crossover_method = (int)integer(argv[8], 0, 3);
    c.population_param.sampling_type = pop_uniform;
    c.population_param.reseed_bottom_N = 1;
    c.fx_param.fx_optim_mode = fx_optim_mode_minimize;
    c.optimizer_param.convergence_window = generations;
    /* Existing solver stops when zero-based iteration > max_iterations.
       Preserve the algorithm and translate to an exact process_pop call budget. */
    c.optimizer_param.max_iterations = generations - 2;
    c.optimizer_param.convergence_moving_window_size = 200;
    c.optimizer_param.convergence_threshold = 1e-20;
    c.optimizer_param.min_mutations = 1;
    c.optimizer_param.max_mutations = 5;
    c.mutation_param.initial_mutation_rate = 3;
    c.flatten_param.flatten_alpha = 4;
    c.flatten_param.flatten_beta = c.flatten_param.flatten_method == flatten_method_sigmoid ? 0.5 : 0.1;

    double lower = 0, upper = 0, reference_point = 0;
    double (*objective)(double*, uint32_t) = NULL;
    if (!strcmp(argv[1], "styblinski_tang")) {
        c.fx_param.fx_method = fx_method_Styblinski_Tang;
        lower = -5; upper = 5; reference_point = -2.903534027771178;
        objective = Styblinski_Tang_fx;
    } else if (!strcmp(argv[1], "ackley")) {
        c.fx_param.fx_method = fx_method_Ackley; lower = -32.768; upper = 32.768; objective = Ackley_fx;
    } else if (!strcmp(argv[1], "griewank")) {
        c.fx_param.fx_method = fx_method_Griewank; lower = -600; upper = 600; objective = Griewank_fx;
    } else if (!strcmp(argv[1], "levy")) {
        c.fx_param.fx_method = fx_method_Levy; lower = -10; upper = 10; reference_point = 1; objective = Levy_fx;
    } else if (!strcmp(argv[1], "rastrigin")) {
        c.fx_param.fx_method = fx_method_Rastrigin; lower = -5.12; upper = 5.12; objective = Rastrigin_fx;
    } else if (!strcmp(argv[1], "schwefel")) {
        c.fx_param.fx_method = fx_method_Schwefel; lower = -500; upper = 500;
        reference_point = 420.9687462275036; objective = Schwefel_fx;
    } else {
        fprintf(stderr, "Unsupported 32-dimensional problem: %s\n", argv[1]);
        free_config_ga(&c); return 2;
    }
    double reference_vector[32];
    for (uint32_t i = 0; i < r.genes; i++) {
        c.population_param.lower[i] = lower; c.population_param.upper[i] = upper;
        reference_vector[i] = reference_point;
    }
    observation_t o = {0};
    o.generations = generations;
    o.interval = integer(argv[9], 1, 100000000);
    o.reference = objective(reference_vector, r.genes);
    o.tolerance = 1e-3; /* absolute objective gap per dimension */
    o.samples = calloc((size_t)(generations / o.interval) + 3, sizeof(sample_t));
    if (!o.samples) return 3;
    r.generation_observer = observe;
    r.generation_observer_context = &o;
    LARGE_INTEGER frequency, begin, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&begin);
    progress_t progress;
    double best = Genetic_Algorithm(c, r, &progress);
    QueryPerformanceCounter(&end);
    double wall = (double)(end.QuadPart - begin.QuadPart) / (double)frequency.QuadPart;
    double verified = objective(o.solution, r.genes);
    if (o.completed != generations || progress.tasks_completed != 1 ||
        !isfinite(best) || !isfinite(verified) || o.seconds <= 0 ||
        fabs(verified - best) > 1e-9 * fmax(1.0, fabs(best))) {
        fprintf(stderr, "Run validation failed: generations=%u expected=%u best=%.17g verified=%.17g\n",
                o.completed, generations, best, verified);
        free(o.samples); free_config_ga(&c); return 4;
    }
    printf("{\"schema\":1,\"problem\":\"%s\",\"population\":%u,\"genes\":32,\"seed\":%u,"
           "\"mutation\":%d,\"selection\":%d,\"flatten\":%d,\"crossover\":%d,"
           "\"generations\":%u,\"evaluations\":%" PRIu64 ",\"compute_seconds\":%.17g,"
           "\"wall_seconds\":%.17g,\"generations_per_second\":%.17g,"
           "\"objective\":%.17g,\"reference\":%.17g,\"gap_per_gene\":%.17g,"
           "\"target_gap_per_gene\":%.17g,\"target_generation\":%u,\"target_compute_seconds\":%.17g,"
           "\"solution\":[",
           argv[1], r.individuals, r.random_seed, c.mutation_param.mutation_method,
           c.selection_param.selection_method, c.flatten_param.flatten_method,
           c.crossover_param.crossover_method, o.completed,
           (uint64_t)o.completed * (r.individuals - r.elitism), o.seconds, wall,
           o.completed / o.seconds, best, o.reference, (best - o.reference) / r.genes,
           o.tolerance, o.target_generation, o.target_seconds);
    for (uint32_t i = 0; i < r.genes; i++) printf("%s%.17g", i ? "," : "", o.solution[i]);
    printf("],\"trace\":[");
    for (uint32_t i = 0; i < o.count; i++)
        printf("%s[%u,%.17g,%.17g]", i ? "," : "", o.samples[i].generation,
               o.samples[i].seconds, o.samples[i].objective);
    puts("]}");
    free(o.samples); free_config_ga(&c);
    return 0;
}
