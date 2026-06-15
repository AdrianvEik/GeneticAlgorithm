# Current Architecture

The current implementation is a C genetic algorithm library. The public entry point is
{c:func}`Genetic_Algorithm`, declared in
[`src/Genetic_Algorithm.h`](../../src/Genetic_Algorithm.h) and implemented in
[`src/Genetic_Algorithm.c`](../../src/Genetic_Algorithm.c).

## Main Runtime Connection

{c:func}`Genetic_Algorithm` connects the main subsystems in this order:

1. Validate {c:type}`config_ga_t` and {c:type}`runtime_param_t` with
   {c:func}`verify_input_parameters`.
2. Optionally export configuration through {c:func}`write_config`.
3. Create a {c:type}`console_queue_t`, {c:type}`task_result_queue_t`,
   {c:type}`task_queue_t`, and {c:type}`fx_task_queue_t`.
4. Start logging, solver, and optional fitness-worker threads.
5. Generate solver tasks through {c:func}`make_task_list`.
6. Stop solver and fitness workers once all tasks have been queued and
   processed.
7. Stop the result logger, close files, free queues, and return the best
   result.

The source comments in [`src/Genetic_Algorithm.h`](../../src/Genetic_Algorithm.h)
describe the public call contract. The queue and worker relationships are
documented in [Threading and Logging](threading_and_logging.md).

## Per-Task Connection

Each solver task owns a {c:type}`gene_pool_t` for the lifetime of the worker
loop. A task passes through these connections:

- {c:func}`fill_pop` seeds chromosomes using {c:type}`population_param_t`.
- {c:func}`process_pop` runs one full genetic iteration.
- {c:func}`adapt_param` updates mutation rates and convergence state.
- {c:func}`report_task` serializes top individuals for the logger.

This keeps population memory local to the solver thread while sharing only
bounded queues between threads.

## Configuration Connection

The shared configuration model lives in
[`src/Helper/Struct.h`](../../src/Helper/Struct.h). It is intentionally grouped
by pipeline stage:

- {c:type}`population_param_t` feeds population seeding.
- {c:type}`selection_param_t` feeds parent selection.
- {c:type}`flatten_param_t` feeds fitness flattening.
- {c:type}`crossover_param_t` feeds chromosome recombination.
- {c:type}`mutation_param_t` feeds mutation.
- {c:type}`fx_param_t` feeds objective evaluation.
- {c:type}`optimizer_param_t` feeds adaptive stopping and mutation updates.

For the details of each struct and function contract, see
[Current API Reference](current_api.rst).
