# TODO

This page collects open ends from the source comments and the current test
surface. It is intentionally small and practical: things here should either
become issues, tests, or code changes.

## Source TODOs

### Algorithm Correctness

- Fitness normalization and selector repairs are implemented; contracts and
  method IDs are recorded in [Operator Pipeline](operator_pipeline.md).
- Preserve the adjacent SIMD dedupe algorithm and existing worst-slot reseed
  policy. Discuss exhaustive equal-score-group detection or reseeding specific
  duplicate positions separately if desired.
- Broader operator validation sweep (explicitly deferred by the user): validate
  active flattener finite alpha >= 0, linear beta >= 0, sigmoid beta in [0,1];
  selection p/lambda in [0,1], positive finite temperature, tournament size >= 1,
  class threshold in [0,1], and supported method/rank-distribution IDs. Extend
  equivalent checks to crossover, mutation, optimizer, and population settings
  together. Currently these are documented preconditions; do not silently alter
  invalid configurations. NaN/infinite objectives remain a separate contract.
- Broader reproducibility/configuration-export sweep (explicitly deferred):
  include rank-distribution choice, Boltzmann class threshold, all effective
  settings, numeric precision, and method semantics. Existing JSON exports have
  not been expanded in this change.
- Reporting snapshots and score/candidate correspondence remain deferred for
  separate discussion. Preserve existing adaptation timing when addressing it.
- Remove the optional mutation/adaptation debug fields from CSV and binary
  solution exports (including header/row sizing and configuration counts).
  These were debugging aids, not solution metadata. Their timing/rank indexing
  does not need a separate redesign before removal.
- Fix and regression-test elite preservation in `process_crossover()` after
  the confirmed reproduction below. Keep this separate from result statistics.
- Compare mixed/strict/relaxed three-candidate Boltzmann and pairwise logistic
  under equal objective-evaluation budgets. Existing regression checks establish
  probability behavior and termination, not optimization superiority.
- Consider phenotype-aware diversity distances (including fixed dimensions and
  categorical genes). Current rank-space distance uses normalized uint32 codes.

- fix 64 bit boundary for complete crossover
- Mutate on full gene boundary as an extra function


### Adaptive Controls

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
- `src/Utility/mutation.c`: validate alignment and vectorization requirements
  for the mutation-rate block now owned by the aligned gene-pool allocation.
- Fitness weights deliberately use [0,1]; review alternative pressure controls
  only as an explicit change to the documented operator contracts.
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

## Results and measurement follow-up (2026-10-07)

The small O9 statistics fixes are implemented: public best/mean, CSV, and binary
scores use original objective units; the logger computes the actual mean and
sample standard deviation with Welford's algorithm once per completed task.
No completions gives NAN best/mean/stddev; one completion gives NAN stddev.
The effective task direction travels with the record because built-in objectives
can override the originally configured direction. Rebuild clients for the
record/queue layout changes and account for the changed minimization sign and
mean/stddev semantics. Internal selection/ranking scores are unchanged.

Remaining bookkeeping should be placed at generation boundaries:

- After `process_fx()` returns successfully in `process_pop()`, increment a
  task-local evaluation count by the number actually evaluated (currently
  `individuals - elitism`). No per-objective atomic counter is needed. If the
  evaluation policy later changes, have the evaluator return that count.
- Immediately after sorting, before crossover/mutation/reseeding, compare the
  generation winner with a task-local best-ever record. Copy its evaluated
  inputs and record its discovery iteration only on strict improvement. This
  costs one comparison per generation and O(genes) only for improvements;
  allocate candidate storage once, not inside the generation loop.
- Capture requested top-N evaluated inputs/scores at that same boundary on
  export generations. Do not snapshot the entire population every generation.
  The saved best-ever record supplies a final result when stopping is detected
  later. Preserve the current variation/adaptation order while separating
  capture time from the later decision to publish a final record.
- Set a termination-reason enum where `check_convergence()` detects a stopping
  condition. Define precedence when multiple conditions fire. The current
  single boolean loses the distinction between convergence and iteration limit.
- Keep final-report iteration separate from best-discovery iteration.
  `progress_t.best_result_iteration` still means final-report iteration;
  candidate return, evaluation count, best-ever tracking, and stop reason are
  not implemented by the small statistics change.

Q4 is data-type dependent: double evaluation writes decoded inputs into a
separate buffer and normal variation leaves those doubles/scores intact, so
evaluated double CSV rows can remain consistent after variation. Integer CSV
reads changed chromosomes directly; binary logging always copies the double
buffer, even for integer objectives. Unevaluated slots and elite preservation
still need explicit treatment. A future result record must capture the actual
evaluated input for either data type before variation.

Elite preservation was verified against the real `process_crossover()` in a
focused probe with 64 individuals, 16 genes, two elites, complete crossover,
and seed 12345. The final elite's active buffer started at 1063 and its alternate
buffer at 9063. After crossover both held 9063; the expected active value was
1063. `memcpy_s` currently has the active buffer as destination and alternate
buffer as source, followed by a pointer swap. This overwrites the elite before
the swap. Crossover was not modified in this investigation. The local probe,
build script, and captured output are under ignored `build/results-audit/`.
