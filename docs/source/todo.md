# TODO

This page collects open ends from the source comments and the current test
surface. It is intentionally small and practical: things here should either
become issues, tests, or code changes.

## Source TODOs

### Algorithm Correctness

- `src/Utility/process.c` and `src/Utility/flatten.c`: separate true fitness
  ordering from selection-pressure scaling. Sort `sorted_indexes` from the
  sign-canonicalized `pop_result_set`, then apply flattening/scaling for
  selection only. This prevents flattening from accidentally redefining the
  true best/worst individual used by elitism, reseeding, mutation pressure, and
  logging.
- `src/Utility/flatten.c`: fix logarithmic and normalized flattening. These
  modes currently read `pop_result_set[0]` and
  `pop_result_set[individuals - 1]` as if the result set were sorted, but
  flattening runs before sorting in the current chain.
- `src/Utility/flatten.c`: check the monotonicity of linear and exponential
  flattening when scores may be positive and negative. Because both divide by
  the total score, a negative or near-zero sum can reverse or destabilize
  selection pressure.
- `src/Utility/selection.c`: fix rank-based selection so sampled ranks are
  mapped through `sorted_indexes`. The thread-local rank distributions should
  select a rank, not be treated as physical individual indexes.
- `src/Utility/selection.c`: define roulette input requirements explicitly.
  Roulette needs non-negative, monotonic cumulative weights; raw `none`
  flattening can violate this if canonical scores are negative.
- `src/Utility/selection.c`: align Boltzmann tournament selection with
  Goldberg's acceptance/anti-acceptance formulation. The implementation should
  map sorted objective or flattened weights into the intended Boltzmann
  distribution, use a temperature/control schedule deliberately, and guard the
  pairwise acceptance probabilities from invalid negative complements.
- `src/Utility/process.c`: decide whether `individuals - elitism` must be even
  and add a validation check or odd-parent handling if so.
- `src/Genetic_Algorithm.c`: revisit thread-local RNG seeding. The solver
  worker currently seeds each thread with `runtime_param.random_seed`, which
  may make workers identical when a non-zero seed is reused.

### Adaptive Controls

- `src/Optimisation/Optimizer.c`: validate the adaptive mutation formula and
  check whether the logged mutation signal is correct.
- `src/Utility/crossover.c` and `src/Optimisation/Optimizer.c`: make
  `crossover_prob` an active control parameter. It should eventually have a
  PID-like adaptive controller similar in spirit to the mutation controller, so
  crossover pressure can respond to convergence and diversity instead of being
  static or ignored.
- `src/Helper/Struct.h`: design per-gene mutation probability and
  rank-dependent mutation pressure instead of using only a per-individual
  mutation-rate array.
- `src/Optimisation/Optimizer.c`: implement or remove the reserved adaptive
  flattening fields and TODO comments.

### Edge Cases And Validation

- `src/Utility/process.c`: handle remaining and small sort sizes in the
  vectorized indexed bitonic sort paths.
- `src/Utility/mutation.c`: confirm whether `mutation_param->mutation_rate`
  should be explicitly aligned or copied to aligned scratch storage.
- `src/Utility/flatten.c`: make the exponential flattening clamp domain
  configurable instead of hard-coding `[0, 1]`.
- `src/Utility/pop.c`: either repair or remove the commented Cauchy population
  path; the comment notes undesirable casts from integer RNG output to double.
- `src/Multiprocessing/mp_logger.c` and `mp_logger.h`: confirm ownership of the
  shared `console_queue_t` pointer and whether progress initialization belongs
  in the result queue.

### Cleanup And Naming

- `src/Utility/selection.c`: evaluate whether roulette selection should use a
  tree/binary search over the cumulative distribution.
- `src/Utility/process.c`: rename dimensions consistently, for example
  `genes` to `gene_count` and `individuals` to `individual_count`, if this is
  still desired.

## Test Coverage To Add

- Algorithm ordering: verify that elitism, reseeding, mutation pressure, and
  logging use true canonical fitness order, while flattening only changes
  selection pressure.
- Flattening monotonicity: linear, exponential, logarithmic, normalized,
  sigmoid, and none modes over positive, negative, mixed-sign, and equal-score
  populations.
- Selection mapping: roulette, tournament, rank, rank-space, and Boltzmann
  selection with deterministic distributions, including rank-to-`sorted_indexes`
  mapping.
- Population allocation and cleanup: `init_gene_pool`, `free_gene_pool`, and
  alignment-sensitive buffer layout.
- Population initialization: uniform and normal sampling paths in `fill_pop`,
  including deterministic RNG tests where possible.
- Fitness evaluation: built-in Styblinski-Tang, Wheeler's Ridge validation, and
  callback-based `fx_method_pointer` for both double and integer data paths.
- Crossover dispatcher: `process_crossover`, including elitism and probability
  behavior, not only the static helper functions. Add coverage for the adaptive
  crossover-probability controller once it exists.
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


- check logging value to individual result is not correct compute result and then log it before processing the population