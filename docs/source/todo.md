# TODO

This page collects open ends from the source comments and the current test
surface. It is intentionally small and practical: things here should either
become issues, tests, or code changes.

## Source TODOs

- `src/Genetic_Algorithm.c`: revisit thread-local RNG seeding. The solver
  worker currently seeds each thread with `runtime_param.random_seed`, which
  may make workers identical when a non-zero seed is reused.
- `src/Helper/Struct.h`: design per-gene mutation probability and
  rank-dependent mutation pressure instead of using only a per-individual
  mutation-rate array.
- `src/Utility/mutation.c`: confirm whether `mutation_param->mutation_rate`
  should be explicitly aligned or copied to aligned scratch storage.
- `src/Utility/flatten.c`: make the exponential flattening clamp domain
  configurable instead of hard-coding `[0, 1]`.
- `src/Utility/pop.c`: either repair or remove the commented Cauchy population
  path; the comment notes undesirable casts from integer RNG output to double.
- `src/Utility/process.c`: handle remaining and small sort sizes in the
  vectorized indexed bitonic sort paths.
- `src/Utility/process.c`: decide whether `individuals` must be even and add a
  validation check if so.
- `src/Utility/process.c`: rename dimensions consistently, for example
  `genes` to `gene_count` and `individuals` to `individual_count`, if this is
  still desired.
- `src/Optimisation/Optimizer.c`: implement or remove the reserved adaptive
  flattening fields and TODO comments.
- `src/Optimisation/Optimizer.c`: validate the adaptive mutation formula and
  check whether the logged mutation signal is correct.
- `src/Utility/selection.c`: evaluate whether roulette selection should use a
  tree/binary search over the cumulative distribution.
- `src/Utility/selection.c`: update Boltzmann selection so it can use the
  Boltzmann distribution directly instead of relying on flattened fitness.
- `src/Multiprocessing/mp_logger.c` and `mp_logger.h`: confirm ownership of the
  shared `console_queue_t` pointer and whether progress initialization belongs
  in the result queue.

## Test Coverage To Add

- Population allocation and cleanup: `init_gene_pool`, `free_gene_pool`, and
  alignment-sensitive buffer layout.
- Population initialization: uniform and normal sampling paths in `fill_pop`,
  including deterministic RNG tests where possible.
- Fitness evaluation: built-in Styblinski-Tang, Wheeler's Ridge validation, and
  callback-based `fx_method_pointer` for both double and integer data paths.
- Flattening: linear, exponential, logarithmic, normalized, sigmoid, and none
  modes, including boundary and clamp behavior.
- Selection: roulette, tournament, rank, rank-space, and Boltzmann selection
  with deterministic probability distributions.
- Crossover dispatcher: `process_crossover`, including elitism and probability
  behavior, not only the static helper functions.
- Mutation dispatcher: `process_mutation`, including sorted indexes, elitism,
  per-individual mutation rates, and zero-mutation behavior.
- Adaptive optimizer: convergence detection, max-iteration stopping, moving
  window initialization, and mutation-rate bounds.
- Task generation: zoned and non-zoned `make_task_list`, including integer
  mask generation and double bound splitting.
- Queue behavior: task queue, fitness queue, result queue, and console queue
  push/pop behavior under empty and full conditions.
- Logging: CSV header generation, binary row sizing, JSON configuration export,
  and best-result tracking.
- Thread wrapper: Windows and pthread wrapper paths, ideally with compile-only
  CI jobs for both platforms.
- End-to-end GA smoke test: a small deterministic optimization run with a known
  seed, small population, and short convergence window.
- CMake/CTest integration: ensure tests link consistently through the CMake
  targets and can run in CI on Windows first, then Linux once the portability
  layer is mature.
