# GA project working notes

Last updated: 2026-10-07 (includes F9/O9 result statistics and Q4/Q8 measurement follow-up, commit `ebe1902`; earlier review validation was performed on 2026-10-03).

## Project overview

- Delivery branch: `fix-up`, created from the review baseline with the changes developed in this session.

- Review baseline: `C-code`, commit `f3872d8`. Work in this branch's C implementation; the earlier review initially inspected an older checkout before switching to this revision.
- Goal: characterize optimization quality, parameter responses, performance, and failure conditions with reproducible experiments.
- Workloads: expensive objectives evaluated concurrently; cheap objectives evaluated inline with independent searches across the full domain or partitioned domains.
- Longer-term goal: compose GA searches, including an outer GA that tunes configurations of inner searches, from a neighbouring C project.
- Current priority: correctness and understanding the existing implementation. Packaging/export and the wider API redesign are deferred.
- Existing foundation: CMake static target `ga`, an executable under `app/`, and operator tests under `Tests/`.
- This file is a handoff record, not a claim that the full solver has been validated. Distinguish source findings, isolated checks, and end-to-end evidence.

## Decisions and points worked out

- **Fitness convention:** `process_fx_set()` multiplies objective values by the optimization direction. Internal ranking always maximizes; minimization evaluations are negated. Unevaluated internal scores must therefore start at the worst maximization score in either mode.
- **Evaluation interval:** `process_fx_set()` uses an inclusive end index. For `N` individuals and `E` elites, the current non-elite interval is `[0, N - E - 1]`. This does not resolve which individuals to evaluate in generation zero.
- **Adaptation timing:** `process_task()` calls `adapt_param()` once per population iteration, after `process_pop()`. Ordinary fitness evaluation does not write the mutation-rate array.
- **Population ownership:** `process_task_thread()` allocates a separate gene pool per solver worker and reuses it across that worker's tasks. Concurrent solver workers do not share this allocation.
- **Mutation-rate ownership:** mutable per-rank mutation rates now belong to the solver worker's gene pool and are reset from the task's immutable scalar starting rate before every task. See F8/O1 below.
- **Cascade status:** the current `optimize_fx_ga()`, scalers, and `weigh_result()` are a disposable WIP proof of concept. The user intends to replace them. Do not treat improving that scoring/decoding scheme as the current objective.

## Fixed in the current changes

- [x] **F10 — Preserve elites across crossover buffer swaps (O16).** Reversed the elite `memcpy_s` source/destination in `process_crossover()` so current elites are copied into the next-population buffer before the existing pointer swap. The permanent `test_crossover_elitism` regression failed on the old code and passes on the fix; the original probe now preserves 1063 instead of replacing it with 9063. See the repair update below.

- [x] **F9 — Objective units and completed-task statistics (partial O9).** Commit `ebe1902` converts reported scores back to original objective units in `report_task()`, including CSV/binary score columns and the public scalar/progress result. Records carry the effective task direction. `record_completed_result()` computes the actual mean and sample standard deviation with Welford's algorithm once per completed task in the logger; its M2 accumulator is separate from public progress. Empty best/mean/stddev and single-result stddev are NAN. Internal ranking scores and variation/adaptation order are unchanged. Candidate capture, discovery iteration, evaluation count, and stop reason remain open; see the detailed update below.

- [x] **F4 — Unused crossover setting removed.** Removed `crossover_stepsize` from `crossover_param_t`; no consumers were found. Rebuild clients because the struct layout changed.
- [x] **F5 — Inline defaults and verified combinations.** `task_size_fx = 0`, `thread_count_fx = 0`, and `write_bin = 0` are explicit defaults. Both fitness settings must be zero for inline evaluation or both positive for queued evaluation. `verify_input_parameters()` rejects each mixed combination with a specific diagnostic and exit 250 before allocating queues or starting threads. Per user direction, settings are never silently normalized. Disabled queues allocate no resources; solver/logger threads still exist. Updated the WIP inner GA to explicitly request zero fitness workers alongside its zero batch size, preserving its intended inline behavior.
- [x] **F6 — Stable fixed-base task RNG streams (O5).** `process_task_thread()` reseeds before each real task through `seed_rand_task_threadlocal(base, task_id)`. For nonzero base seeds, unsigned addition wraps modulo 2^32 and derived zero directly initializes SFMT without requesting entropy. A base seed of zero retains automatic seeding, now per task. Verified replay after other tasks/random draws, execution on another worker, and wraparound against direct SFMT initialization. Full GA repeatability still depends on objective behavior and the remaining state audits.
- [x] **F7 — Atomic fitness completion flags (O2).** `fx_ready` elements are C11 atomics. `process_fx()` resets every non-elite flag before publishing the first batch, workers publish completed score writes with release stores, and the solver polls with acquire loads. The packed allocation uses the atomic element size and explicitly initializes every flag. MSVC builds enable `/experimental:c11atomics`. A threaded regression starts from stale ready flags, delays the final individual, and verifies that `process_fx()` does not return early. The existing `Sleep(10)` polling remains O7; the built-in direction write is tracked separately under O8.
- [x] **F8 — Gene-pool-owned mutation rates (O1).** The mutable per-rank array moved from `mutation_param_t` into the gene pool's aligned allocation. Configuration now stores only the immutable scalar `initial_mutation_rate`, defaulting to 6.0. `process_task()` calls `init_mutation_rates()` before population initialization, so worker-reused storage is reset at every task boundary. Adaptation, mutation, CSV logging, and binary logging read or write the active gene pool; binary logging now copies the selected rate value rather than pointer bytes. `free_config_ga()` no longer frees mutation state. Rebuild clients because the public structs and configuration field name changed.

- [x] **F1 — Fitness batch boundary.** In `src/Function/Function.c::process_fx()`, clamp an inclusive batch end that reaches or exceeds `N - E` to `N - E - 1`. This excludes the first elite and avoids that one-past-population endpoint when elitism is zero. The subtraction is inside the non-empty scheduling loop. Completion synchronization was addressed separately by F7.
- [x] **F2 — Initial score sign.** In `src/Utility/pop.c::fill_pop()`, replace the minimize/maximize sentinel conditional with `-DBL_MAX` for both modes. These are internal fitness scores, already in the maximization convention. This fixes the sentinel sign; generation-zero evaluation remains O3.
- [x] **F3 — Mutation slope arithmetic.** In `src/Optimisation/Optimizer.c::compute_mutation_rate()`, cast both `i` and `individuals` to `double` before division. Previously every ratio in the loop was zero. Retain the existing `i / N` definition; choosing `i / (N - 1)` would be a separate policy decision.

## Findings and remaining work

- [x] **O1 — Shared mutable mutation rates (F8).** Concurrent solver workers now have distinct arrays because mutation rates live in their separate gene pools. A worker may reuse its pool for successive tasks, but `init_mutation_rates()` restores every entry from `task->config_ga.mutation_param.initial_mutation_rate` before the new population is filled. The configuration is no longer modified by adaptation, and no mutation allocation is copied through `init_task()`.
- [x] **O2 — Fitness completion protocol (F7).** Every active flag is now reset before jobs are published. Atomic release stores publish each completed score and acquire loads prevent the solver from continuing until all required results are visible. The stale-flag and plain-integer data races are covered by a delayed-worker regression. This retains polling rather than adding an event-based barrier; polling overhead remains O7.
- [x] **O3 — Full finite fitness range (operator implementation).** Mandatory normalization, all flatteners and selectors repaired for finite scores under documented parameter preconditions. Exact endpoint policy and validation evidence are in the O3/O11 implementation update below. Invalid/non-finite input contracts and broad validation remain O13.
- [ ] **O4 — Remaining internal initialization audit.** All four configuration-default omissions are resolved by F4/F5: three fields now default to zero and unused `crossover_stepsize` is removed. F9 initializes all progress fields, including elapsed time and the empty best-iteration value. The separate internal initialization inventory below remains open for CSV sizing and integer zone masks.
- [x] **O5 — Fixed-base seeds now belong to tasks (F6).** Real tasks reseed their solver TLS RNG before population initialization using `(base_seed + task_id) mod 2^32`. Derived zero is a deterministic SFMT seed. Base zero still requests automatic seeding per task; persisting automatic seeds is deferred. This fixes RNG stream assignment but does not control random external objectives.
- [ ] **O6 — Inline fitness is not a fully serial solve.** Both fitness settings zero disable fitness workers and their queue (F5); mixed zero/positive settings are invalid. Solver and logging threads still run. Preserve the distinction between an inline CPU evaluation backend and a fully serial GA implementation; discuss whether the latter is needed.
- [ ] **O7 — Worker polling overhead.** Queue operations use `Sleep(1000)`, evaluation completion uses `Sleep(10)`, and logging uses `Sleep(500)`. These delays can dominate cheap runs. Separate algorithm quality measurements from scheduling overhead; consider condition/event-based waits and reusable workers.
- [ ] **O8 — Objective dispatch is coupled to experiments.** Benchmark functions, GA tuning, decoding, scoring, and dispatch live in `Function.c`. The callback lacks user context and returns no evaluation status. Wheeler's Ridge has a method constant and implementation but no dispatch branch. Built-ins also override the direction during evaluation; queued workers therefore still write the task's direction concurrently. Determine built-in direction before publishing jobs. Resolve objective contracts before expanding the API.
- [ ] **O9 — Result/statistics contract (partially fixed by F9).** Public/exported scores now use original objective units, `average_result` is an actual mean, and `result_standard_deviation` is a computed sample standard deviation. `progress_t` still cannot return a best-candidate vector or evaluation count/stop reason. `best_result_iteration` remains the winning task's final report generation, not discovery time; best-ever tracking without elitism and Q4 candidate correspondence remain open.
- [ ] **O10 — Cleanup across repeated runs.** `start_threads()` allocates a worker-parameter array through a local pointer that is not returned for cleanup. `Genetic_Algorithm()` does not call `free_task_result_queue()`. Queue cleanup also needs an ownership audit for allocated locks. Repeated/cascading runs need bounded memory use and complete teardown.
- [x] **O11 — Selection assumptions (operator implementation).** Repaired probability construction, rank mapping/ties, diversity mixture, and temperature-based selectors. Added strict/relaxed and pairwise variants while preserving existing IDs. See O3/O11 implementation update; optimization-quality comparisons remain open.
- [ ] **O12 — Domain/task generation.** Integer evaluation applies `zone_mask` and `zone_id`, but the non-zoned task path does not initialize those allocated arrays. Integer splitting can shift by 32 for an unsplit dimension. Audit masks, shifts, actual generated task counts, and complete domain coverage before comparing partitioning against restarts.
- [ ] **O13 — Public error handling and validation.** Most failures call `exit()`, terminating an entire experiment sweep. Input validation covers only a few fields. Define behavior for invalid bounds, unsupported dimensions/population sizes, callback failures, NaN/infinity, and no valid candidates. Preserve failing cases as explicit outcomes in experiments.
- [ ] **O14 — Bitonic sorters overrun partial blocks.** `process_pop()` always calls `indexed_bitonic_sort_8v()`. Its full-block loop runs while `i < size` but unconditionally loads and stores 64 elements, so the current `app/ga_exe.c` population of 16 accesses indexes 0 through 63. This corrupts arrays following `sorted_indexes`; the resulting invalid indexes can then fault in `post_bitonic_merge()` when used to index the result array. The defect predates the current branch changes, but packed-layout changes can alter or expose the undefined behavior. The 2v, 4v, 8v, and 16v loops and remainder thresholds (`>= 7`, `>= 15`, `>= 31`, and analogous cases) need a complete block-boundary audit. Add exact-size, undersized, odd, and non-block-multiple regression tests rather than testing only size 128. A population of 64 avoids this specific 8v overrun only as a temporary workaround.

- [ ] **O15 — Automatic task seeds collide within a second.** Confirmed on 2026-10-07 with the current app configuration (32 tasks, 8 solver workers, 128 individuals, 32 genes, inline fitness, `random_seed = 0`). `is_RDRAND_supported()` in `src/Helper/rng.c` accepts only the Intel vendor string; this machine reports `AuthenticAMD` and CPUID RDRAND support, but the function rejects it. The fallback `random_int32()` calls `srand(time(0))` on every seed request. Tasks starting within the same second receive the same seed and repeat the same search. F6's move from once-per-worker to once-per-task seeding exposes this pre-existing fallback defect at every task boundary; its fixed nonzero-base path remains distinct by task ID. Correct automatic entropy and its capability detection/initialization before relying on zero-seeded restarts. No RNG or reporter implementation was changed in this investigation.

- [x] **O16 — Crossover overwrites elites from the alternate buffer (F10).** The reproduced defect replaced active elite 1063 with alternate-buffer contents 9063. Fixed by copying active elites into the alternate buffer before swapping pointers. The original probe and permanent regression now pass. This repairs elite preservation for both data types without changing crossover generation or adaptation timing; Q4 integer reporting still needs its separate capture fix.

### O15 reproduction and completion-report audit (2026-10-07)

- Read the existing `C:/temp/GA.csv` matching the user's console output and preserved it under ignored `build/report-audit/user-run.csv`. The first 32 records contain each task ID 0 through 31 exactly once. IDs 0–7 stop at 2938, 8–15 at 4505, 16–23 at 3689, and 24–31 at 6880. Within each group, every CSV field except task ID is identical, including exported genes and scores. This is repeated search work, not merely identical printed iteration numbers.
- Source trace: `process_task()` creates and resets its own stack-local `adaptive_memory`; each worker has its own gene pool, resets the iteration counter, and dequeues each solver task separately. A convergence result is copied into the result queue once, followed by breaking that task's loop. The logger increments `tasks_completed` by one for each `BEST_RESULT_TASK`. It does not mark other tasks owned by the worker complete. `task_size_fx = thread_count_fx = 0` bypasses fitness queues/completion flags entirely in this configuration. The groups of eight match the solver-worker count, not a fitness batch size.
- Fresh MSVC 19.51 Debug library build in `build/report-audit/cmake`, with tests disabled for this focused investigation. A probe copied the current app configuration and changed only the output basename (to preserve the user's output) and made the seed a command-line argument. Automatic seed 0 reproduced four groups of eight at iterations 3809, 5349, 5862, and 6355; all exported fields except task ID matched within each group. With seed 12345, the same configuration produced 32 distinct stopping iterations and 32 distinct exported records. Both runs contain all 32 task IDs exactly once before the final best-record duplicate.
- A separate probe linked the freshly built RNG and called the actual `seed_rand_task_threadlocal(0, id)` for eight IDs within one second. All eight produced the same first four SFMT outputs. It also recorded CPUID RDRAND=1 versus the library's detected support=0 on this AMD CPU.
- The CSV has 33 data rows because logger shutdown deliberately writes the saved overall best record once more. That extra row is not another completion event and does not increment progress. This is separate from the eight-task clusters.
- Immediate experimental workaround: use an explicit nonzero `runtime_param.random_seed`, retaining the existing `base_seed + task_id` mapping. A proper automatic-seeding repair remains open. Ignored probes and captured output live in `build/report-audit/`; the existing app edits were preserved. This audit does not validate the other open optimizer/operator issues.

## Questions and proposals requiring more reasoning

- [x] **Q1 — Ownership choice for O1.** Chosen design: the worker-owned gene pool holds the mutable, individual-sized rate array and gives its active task exclusive use. Configuration holds the immutable scalar starting rate. Rates reset at every task boundary, and nested solves remain isolated when they run through their own solver worker and gene pool.
- [ ] **Q2 — Evaluation semantics.** Retain intentional partial initial evaluation and make operators robust to the full finite fitness range (O3). Decide separately whether unevaluated sentinels need explicit metadata to distinguish them from legitimate worst-case objective values. Establish elite reuse for noisy objectives and behavior on evaluation failure. Use a generation barrier before selection/adaptation.
- [ ] **Q3 — Mutation controller behavior.** After F3, inspect response curves, denominator-zero cases, min/max limits, rank direction, and whether fixed mutation should be available as a baseline. The casts do not validate the whole controller.
- [ ] **Q4 — Measurement timing.** Reporting follows variation, but the mismatch is data-type dependent: evaluated double inputs remain in a separate decoded buffer, so double CSV rows can still match scores. Integer CSV reads changed chromosomes; binary logging copies the double buffer even for integer objectives. Capture evaluated inputs/scores after sorting and before variation, while preserving operator/adaptation logic. Unevaluated slots remain a separate concern; O16 elite preservation is repaired by F10. No snapshot implementation is included in F9/F10.
- [ ] **Q5 — SIMD and small sizes.** Establish supported population/gene sizes, alignment/tail handling, and CPU instruction requirements. The code forces AVX feature macros; build presets alone do not establish portability. Include a scalar reference or explicitly validate supported limits.
- [ ] **Q6 — Search and execution controls.** Separate independent searches, evaluation workers, evaluation batch size, restarts, and explicit subdomains. Define nested parallelism and worker budgets before reusing pools for cascades.
- [ ] **Q7 — Future objective API.** Consider typed read-only callbacks with user context, evaluation status, and a thread-safety/lifetime contract. Distinguish raw `uint32_t` chromosomes from bounded integer decision variables. Move benchmark objectives and tuning code into examples/experiments.
- [ ] **Q8 — Future results/observers.** Original objective values are implemented by F9. Best candidates, evaluation counts, discovery iteration, termination reasons, seeds, and broader timing/effective configuration metadata remain future work. Observe evaluated inputs before variation and let logging consume those observations. User clarified that mutation/adaptation export fields are debugging aids, not solution metadata; removal is a TODO rather than a priority to repair their timing/indexing.
- [ ] **Q9 — New cascade design.** Decode named integer/real/categorical parameters with explicit constraints. Evaluate inner configurations over specified problems and seeds under fair budgets. Start with normalized final error under a fixed evaluation budget; report success, time, and variability separately. Validate selected configurations on held-out seeds/problems.

## Discussion details: O2–O5

### O2: why a batch could appear complete early

After an evaluation, flags for a four-individual batch are `[1, 1, 1, 1]`. Next generation resets only its first flag: `[0, 1, 1, 1]`. The worker finishes the first individual and writes its flag, producing `[1, 1, 1, 1]` while the other three are still being evaluated. Once other batches also appear complete, the solver can sort, select, mutate, or release task data that workers are still using. Sorted indexes themselves can change underneath the worker's loop.

There was also a separate synchronization problem: ordinary integer flags did not publish result writes safely between threads. The queue mutex protects enqueue/dequeue operations, not subsequent evaluation completion. `Sleep()` or `volatile` would not have established the missing synchronization.

Implemented protocol (F7): before publishing any jobs for a generation, reset every active per-individual atomic flag. Each worker writes an individual's result and then publishes completion with a release store. The solver checks every required flag with acquire loads before using the results. This is the atomic per-individual option discussed here; an event-based pending-batch barrier remains a possible replacement if polling overhead is addressed under O7.

Determine built-in objective direction before enqueueing evaluation jobs; workers should only read this task setting. Writing the same direction value concurrently is still a data race.

### O3: evaluate fewer starting candidates deliberately

The policy can be understood as starting with `N - E` evaluated candidates and growing into a population of `N`. This can be reasonable for expensive objectives and small `E / N`. For `G` evaluation rounds, it saves `E` calls relative to full initial evaluation followed by the same policy, out of roughly `G * (N - E) + E` calls. Neither the benefit nor a negligible effect on optimization quality has been measured.

Current requirement: all operators must tolerate any finite fitness in `[-DBL_MAX, DBL_MAX]`, including initial sentinels. Signed scores must be transformed safely where nonnegative probabilities are required, with defined behavior for equal/all-zero weights. Explicit sentinel exclusion is an optional semantic decision, not an agreed implementation. Enough candidates must be evaluated to establish elites, and elitism must preserve scores and chromosomes together. `E == N` evaluates nobody; if all `E` elite slots must contain distinct evaluated candidates after the first pass, the design needs `N - E >= E` or an explicit bootstrap rule.

While tracing that last condition, `process_crossover()` was found to copy the elite cross-buffer into the current population before swapping buffers. The intended preservation appears to require the opposite copy direction. Record this as an additional source finding to verify with an elite chromosome/score regression test; no crossover change has been made in this discussion.

### O4: constructor inventory for tracing

These were all unset active fields found in the default configuration constructors. They are now resolved by F4/F5:

| Field | Constructor | Resolution |
| --- | --- | --- |
| `runtime_param_t.task_size_fx` | `default_runtime_param()`, `src/Helper/Struct.c` | Defaults to 0 (inline). |
| `runtime_param_t.thread_count_fx` | `default_runtime_param()` | Defaults to 0 (no fitness workers). |
| `logging_param_t.write_bin` | `default_logging_param()` | Defaults to 0 (off). |
| `crossover_param_t.crossover_stepsize` | `default_config()` | Removed; no consumer existed. |

Additional internal initialization paths found during this discussion (a separate inventory, not a proof of an exhaustive whole-program read-before-write audit):

- `task_result_queue_t.csv_single_entry_length`: assigned only when CSV is enabled, but read unconditionally by `init_task_result()` to compute `csv_buffer_length`.
- `progress_t.best_result_iteration`: resolved by F9; initialized to UINT32_MAX when empty, then assigned the winning task's final reporting iteration. Discovery iteration remains O9.
- `progress_t.elapsed_time`: resolved by F9; zero-initialized with the rest of progress, then updated by the logger.
- `task_param_t.zone_mask[]` and `zone_id[]`: allocated by `init_task()` but not filled in the non-zoned path; integer objective evaluation subsequently reads them (O12).
- `fx_task_param_t.task_id`: never assigned for fitness jobs; no consumer found. `thread_param_t.status` is first assigned on task completion; no reader found. These are unused/incomplete metadata rather than demonstrated evaluation failures.
- Some fields are intentionally initialized later: solver task IDs by `add_task()`, iteration numbers by the solver loop, console message count by `Genetic_Algorithm()`, and file handles only for enabled outputs. Do not label every such field a bug.
- `fx_ready[]` is explicitly initialized and reset with atomic operations; release/acquire synchronization is implemented in F7/O2.

### O5: stable task seeds

The user proposed `base_seed + sequential_task_number`. Implemented in F6: the single producer assigns `task_id` monotonically in `add_task()` before publishing each task. The solver reseeds its TLS RNG after dequeuing each real task and before `fill_pop()`.

Previously seeding happened once per worker, so tasks inherited whichever worker stream scheduling assigned them. The new fixed-base mapping gives the same logical task the same initial stream independently of prior draws or worker assignment. Task order and configuration still define task identity; changing the partition plan changes which problem a task ID represents.

Nonzero base seeds use addition modulo 2^32, including zero as a deterministic derived SFMT seed. Base zero still requests entropy on each task; resolving and persisting a single automatic run seed is deferred. Distinct task IDs across the 32-bit range give distinct seed bit patterns for a fixed base, but statistical independence is not established. This change does not control stochastic external objectives.

## O3 operator audit: full finite fitness range

Scope: finite objective scores, including `-DBL_MAX` and `DBL_MAX`, under sensible operator parameters. Desired behavior includes finite outputs, preference consistent with larger canonical fitness, and valid selected indexes. Nonnegative probability weights and a positive total (or a documented fallback for ties/zero weights) are requirements for probability-based selectors, not for raw objective values. Saturation can create ties; decide separately how much discrimination to preserve. No operators were repaired in this audit.

Flattener evidence: called the actual `process_flatten()` from the MSVC Debug library for all six methods on five populations: extremes `[-DBL_MAX,-1,1,DBL_MAX]`, negatives `[-4,-3,-2,-1]`, all zero, permuted `[2,4,1,3]`, and four `DBL_MAX` values. Used alpha=1, beta=0. These are counterexamples and smoke checks, not exhaustive proofs.

| Operator | Status and concrete failures | Follow-up |
| --- | --- | --- |
| Linear | **Fails.** Zero/cancelling sums give NaN/infinity. Negative sums reverse preference: negatives produce `[.4,.3,.2,.1]`. Large positive sums overflow and can yield all-zero weights. | Replace unsafe sum normalization; handle signs, scaling, and ties explicitly. |
| Exponential | **Fails.** Same sum/sign problems; exponentiation and normalization produce NaNs for extremes/all-zero input. Negative populations reverse preference. | Define stable, order-preserving scaling before exponentiation; guard exponent range and normalization. |
| Logarithmic | **Fails ordering/range semantics.** Assumes physical array endpoints are extrema, although sorting updates indexes only; also labels last as min and first as max. Negative populations and permuted populations reverse/misorder preferences. Opposite extreme subtraction overflows; `fmax` can hide NaNs as the log floor and produce negative weights. | Find true extrema via indexes/scan; use scaled differences; define valid weight output and equal-score behavior. |
| Normalized | **Fails.** Same endpoint/index issue; equal scores divide by zero, opposite extremes overflow range/subtraction. Permuted `[2,4,1,3]` produced `[1,-1,2,0]`. | Correct extrema; compute normalized differences without overflowing; define ties. |
| Sigmoid | **Conditional.** Default alpha/beta produced finite, nondecreasing outputs in the five cases. `exp` can still overflow internally and saturate; sufficiently negative inputs all become zero, invalid as a roulette total. Extreme offsets/scales need parameter rules and safer evaluation. | Stable sigmoid branches; explicit parameter limits and zero-weight fallback in consuming selectors. |
| None | **Preserves finite scores and ordering.** Safe as a copy over the requested range; does not turn negative/extreme scores into probabilities. | Selectors accepting raw signed fitness must perform any required safe weight conversion themselves. |

Selection audit (all five public methods):

| Operator | Status and concrete failures | Follow-up |
| --- | --- | --- |
| Roulette | **Fails.** Negative weights break cumulative monotonicity. A deterministic probe with `[-4,-3,-2,-1]` returned index 4 for a population of 4. Four `DBL_MAX` weights overflowed the cumulative total. All-zero input degenerates to index 0. | Safely derive nonnegative weights from finite fitness, scale before summing, define zero-total fallback, and enforce output index bounds. |
| Rank tournament | **Core comparison tolerates finite extremes.** A deterministic tournament spanning the extreme test values selected the maximum. The first-draw branch handles scores below -1 correctly. Requires nonzero tournament size and population; can still consume bad upstream flattened values. | Validate settings; test ties and upstream contracts. |
| Rank | **Range-independent idea; implementation fails rank semantics.** Sampled ranks are returned as physical indexes without mapping. Probe with sorted indexes `[2,0,3,1]` and rank-0-only weights returned physical 0. `compute_distr()` uses unsigned `i-1`; at i=0 this underflows, giving zero first weight for p=.2. Rank direction must also match the ascending sort. | Fix mapping, distribution exponent/direction, and probability/temperature validation. |
| Rank-space | **Fails.** Adds signed fitness to distance-derived terms without ensuring valid weights. Distance calculation casts chromosome-scale values to `int` and squares an `int`, risking conversion/overflow; combined values can overflow. Sampled positions also need mapping back to their physical indexes. | Use bounded/wide distance arithmetic, safe score combination, correct index mapping, and roulette safeguards. Source findings, not a runtime overflow test. |
| Boltzmann | **Fails.** Score differences can overflow; exponentials can overflow. Even scores 1 and 0 at T=10 produce probability 1.10517 and complement -0.105171. The branch test `1 / 2` uses integer division, making the intended coin flip always false. | Revisit acceptance formula, stable arithmetic, temperature validation, and randomized branching. |

Important distinction: finite inputs do not imply finite sums, differences, or exponentials. The complete pipeline is not fixed merely by assigning a negative initialization sentinel.

### O3 architecture discussion (2026-10-07; design only)

User decisions and constraints:

- Keep operators acting on the population's base arrays. Make normalization mandatory before optional pressure shaping. Preserve genes and canonical fitness during normalization, flattening, and selection; these operations must not require objective reevaluation.
- Preserve the existing ascending convention throughout: rank 0 is worst, rank N-1 is best, and a sampled rank r maps directly to sorted_indexes[r]. Do not introduce a reversed public/internal rank convention.
- Retain canonical fitness for reporting and other consumers that need original score units. Canonical fitness is direction-adjusted; the original objective value is recoverable using the evaluation's direction, provided that direction is retained.
- Adopt the rank/diversity probability mixture below. Keep the existing three-candidate Boltzmann idea available for repair and compare it with a separate two-candidate logistic method; do not silently replace the existing method or renumber existing method IDs.

Proposed implementation, not yet applied:

- Existing pop_result_set supplies canonical f; existing flatten_result_set can supply selection weights w. Add normalized_result_set for mandatory u in [0,1], inside the worker-owned packed gene-pool allocation, including size/pointer accounting. This is one additional N-double array (8N bytes on the current platform). Keep selection_temp as sampler workspace. All value arrays use physical individual indexes.
- Add a normalization operator with safe extreme-range arithmetic and explicit equal-score handling. Invoke it after evaluation and canonical ranking, before process_flatten. Flatteners read u and write finite nonnegative w; scale w by a common positive factor if a bounded final array is required. Do not perform another min-max subtraction after shaping, because that changes intentional baseline weights and probability ratios.
- Under mandatory normalization, none copies u to w. The existing normalized method becomes an equivalent identity operator after normalization; preserve its ID for compatibility. This replaces the earlier proposal for none to feed raw signed scores to roulette.
- Add a shared safe weighted sampler: bounded cumulative arithmetic, half-open draws, strict cumulative boundary selection, uniform zero-total fallback, and valid physical output indexes. Rank methods sample ranks and then map them; other selectors already construct weights by physical index.
- Normalization provides a consistent tournament input, but does not itself improve comparison-based selection. Proposed tournament policy: compare u, resolve normalization-induced equalities using canonical f, and randomize genuine ties. Flattening is bypassed. This tie-resolution policy remains a proposal.
- Canonical preservation needs a separate duplicate/reseed flag or index list: dedupe_population currently changes pop_result_set using nextafter. Remove that use of scores as bookkeeping when implementing the contract. Also preserve score/candidate correspondence: reporting currently follows variation, so snapshots or a pre-variation observation stage are needed for an exact evaluation record (Q4/O9). Evaluation validity/identity metadata is distinct from fx_ready completion flags; the sentinel policy remains undecided.
- Population-relative normalization preserves gap ratios in exact arithmetic but can collapse distinct floating-point scores in the presence of extreme sentinels. It also changes absolute temperature/threshold units across generations. Store canonical values and define controls in normalized units; do not claim original-unit Boltzmann equilibrium guarantees for changing normalized landscapes.

Accepted rank-space probability equation (physical index i):

    P(i) = (1 - lambda) * P_rank(i) + lambda * P_diversity(i),  0 <= lambda <= 1
    P_rank(i) = R_i / sum_j R_j
    P_diversity(i) = D_i / sum_j D_j

Each component is normalized independently, with uniform probability when that component's weights are all zero. Proposed D_i is bounded mean squared distance from the population centroid in normalized decision coordinates, using an appropriate metric for the gene representation. An implementation can choose the rank sampler with probability 1-lambda or the diversity sampler with probability lambda, which realizes the same mixture without constructing another combined array.

Boltzmann findings and comparison proposal:

- Current exp((score_a-score_b)/T) is used as a probability and complemented by 1-p; for score_a=1, score_b=0, T=10 this gives p about 1.10517 and a negative complement. The strict/relaxed branch uses integer 1/2, so strict is never selected. Candidate selection uses physical-index offsets rather than fitness-class separation; the second can equal the first, and the strict branch does not enforce its documented exclusions. N=2 makes the distance modulus zero. Temperature and candidate availability need explicit rules.
- Checked Goldberg's original 1990 paper, section 3 and the Pascal discussion: https://wpmedia.wolfram.com/uploads/sites/13/2018/02/04-4-5.pdf . Its strict/relaxed distinction concerns objective-value classes, with bounded retries/fallback when suitable classes cannot be found. Its secondary anti-acceptance favors the weaker contestant; the primary acceptance favors the stronger contestant. The current code uses the same preference direction in both stages.
- For normalized maximizing scores, the intended probability signs translate to P(keep B in anti-acceptance against C) = sigmoid((u_C-u_B)/T), followed by P(keep A against secondary winner W) = sigmoid((u_A-u_W)/T). Use a stable logistic helper and positive finite T. Exact threshold and fallback policies still need specification before implementation.
- Keep a separate proposed pairwise comparator P(A beats B) = sigmoid((u_A-u_B)/T). Compare selection-only behavior on fixed populations first, then optimization quality/diversity and cost under equal objective-evaluation budgets and repeatable seeds. No superiority or performance result is established.

No executable code was changed or operator tests run for this discussion; O3/O11 remain open.

## Evaluation backend intent and measured dispatch cost

- User intent: keep GA evolution in C and abstract objective computation so CPU, GPU, NPU, FPGA, or other backends can evaluate candidates. Preserve this separation when redesigning execution and TLS ownership.
- Current queue jobs copy a 32-byte descriptor on this Windows x64 build: kind/ID/range plus pointers to the population and task. Workers are created once per `Genetic_Algorithm()` invocation, not per fitness call. Queued batches reuse them. Numbers/results remain in shared host memory.
- Current inline evaluation already exists (`task_size_fx == 0`). Calling a normal objective inline does not inherently invalidate solver TLS. A nested GA would conflict if it reinitialized outer state on the same thread; the current `Genetic_Algorithm()` starts new solver threads even when called inline, which isolates their TLS. Moving evaluation to a worker isolates that caller's TLS but does not by itself initialize any GA-specific TLS an arbitrary objective might use there.
- Dispatch timing probe: used actual `add_fx_task()`/`get_fx_task()` with one worker and no objective work. Signalled worker entry, waited 50 ms to allow it to enter the empty-queue polling sleep, timed enqueue through dequeue/thread join. Three samples: **948.940, 964.869, 950.371 ms**. This measures an intentionally idle worker wakeup plus join, not steady-state throughput or a typical objective workload. It confirms the `Sleep(1000)` path is material. Completion polling elsewhere adds `Sleep(10)` waits; no new threads per evaluation are needed to cause this latency.
- A small descriptor does not establish negligible overhead. Compare queue/wakeup/completion cost with actual objective time and batch size. For illustrative arithmetic only, 0.95 s overhead against a 0.1 s computation is about 90% of combined time; against 100 s it is under 1%. Parallel overlap changes end-to-end throughput. No heavy-objective or device benchmark was run.
- Future backend contract should represent immutable candidate batches, result buffers, completion/error status, and buffer lifetime. Device adapters need explicit host/device transfer or shared-memory contracts; current host pointers cannot simply be sent as usable addresses to arbitrary hardware. CPU inline and queued workers can be two implementations of the same contract. GPU/NPU/FPGA support has not been implemented or performance-tested.

## Experiment and test backlog

- [ ] End-to-end minimization/maximization tests, first-generation evaluation, bounds, elitism, and score/candidate correspondence.
- [ ] Multi-generation threaded evaluation tests with batch sizes 1, non-divisors, oversized batches, zero elites, and controlled worker delays. Include invalid settings and arithmetic limits.
- [ ] Run-isolation tests: mutation rates, RNG streams, repeated runs, concurrent searches, and nested calls.
- [ ] Operator behavior over positive, negative, mixed-sign, and equal fitness; selection mapping and actual parameter influence.
- [ ] Objective failures and non-finite values, allocation/cleanup checks, and validation of small/odd/tail sizes.
- [ ] Benchmark families: smooth, multimodal, flat, discontinuous, noisy, and poorly scaled problems over several dimensions.
- [ ] Compare final error, success probability, and evaluations to target across explicit seeds. Include uniform random search and fixed-versus-adaptive mutation baselines.
- [ ] Compare partitioned searches with full-domain restarts at equal total evaluation budgets. Explore individual parameter responses first, then interactions.
- [ ] Measure throughput and overhead separately across objective cost, worker count, and batch size. Use a monotonic elapsed-time source and identify hardware/build configuration.

## Deferred work

- Packaging/export: public header independent of scheduler/logger internals; namespaced CMake options; exported `ga::ga` target; install support; external-consumer build test. The existing `ga` target is a starting point.
- Repairing the current cascade. Historical arithmetic probes found its intended `100..30000` range became `37..100`, intended `1..30` could produce `32`, and inverse-iteration scoring used integer division. Keep these as lessons for the replacement, not an active repair list.
- A universal optimizer guarantee. The desired evidence is an empirical operating range: defined problem families, dimensions, accuracy targets, budgets, and success rates.

## Validation record

- F8/O1: MSVC 19.51 x64 Debug complete build succeeded with existing warnings. `test_runtime` verifies distinct rate storage for two gene pools, independent values, adaptation confined to the active pool, unchanged configuration, and per-task reset to the configured initial value. The runtime test and both invalid fitness-setting tests passed through CTest. The crossover, mutation, and sort CMocka executables passed when run directly with their DLL directory available; CTest still reports the existing missing-CMocka-DLL error (`0xc0000135`) for those three registrations. `git diff --check` passed.

- F5 follow-up: replaced silent normalization with verifier errors as requested. MSVC Debug library rebuild and focused runtime checks passed. Both mixed fitness combinations were executed as separate processes and checked through `Tests/expect_runtime_error.cmake`: exact exit code 250 plus the corresponding diagnostic. Both-zero and both-positive settings pass validation. These two error cases are registered with CTest; the scripts were run directly in this validation. Full solver/CMocka tests were not run. The earlier mixed-setting queue test was removed because those settings are now invalid.

- F7: MSVC 19.51 x64 Debug complete build succeeded with C11 atomics enabled. `test_runtime` passed the atomic allocation/initialization checks and a queued regression using ten individuals, batches of four, stale ready flags from a simulated preceding generation, and a delayed final evaluation. The crossover, mutation, and sort CMocka executables also passed when run with their DLL directory available. CTest itself reported those three executables as missing the CMocka DLL (`0xc0000135`); the runtime test and both validation-error tests passed through CTest. Existing compiler warnings remain.

- F4/F5/F6: MSVC Debug library rebuild succeeded with existing warnings. Added durable `Tests/test_runtime.c` and registered `test_runtime` with CTest. Compiled and ran that test directly against the library: default zeros, disabled queue initialization/cleanup for zero settings, normal queue roundtrip, task-stream replay after other draws, execution on another worker, and wrapped seeds all passed. The full CMocka/CTest suite and complete solver were not run; the CTest registration was not exercised through a fresh test-enabled configure.
- O3 audit: 30 actual flattener evaluations (six methods by five inputs) plus deterministic selection probes produced the counterexamples recorded in the operator tables. Probes are local ignored artifacts under `build/review-probes/` (`flatten-audit.c`, `selection-audit.c`). Rank-space findings were established by source inspection. These investigations intentionally expose existing failures, not passing regression tests for repaired operators.
- Dispatch probe: actual queue implementation, three idle-worker samples using Windows performance counters, recorded above. Local artifact `build/review-probes/dispatch-audit.c`; no objective cost or device speedup inferred from this experiment.

- Earlier review: source inspection plus a standalone C arithmetic probe. No full solver benchmarks or end-to-end pass was claimed.
- Current F1/F2/F3 changes: MSVC 19.44 x64 Debug build of the complete `ga` static library succeeded (`BUILD_TESTING=OFF`, `BUILD_GA_EXE=OFF`). Existing compiler warnings remain. The default CMake on PATH was too old for the VS 2022 generator, so validation used Visual Studio's bundled CMake 3.31.6. MSBuild required access outside the sandbox for Windows SDK discovery.
- A local C harness tested the actual `Function.c` with a synchronous mock of queue submission, and linked `fill_pop()` and `adapt_param()` from the built library. Passed 88 evaluation cases: both directions; elitism 0, 3, 31, and 32 out of 32; and batch sizes 0 (inline), 1, 2, 3, 4, 8, 29, 30, 31, 32, and 64. Checked exact evaluation counts and preservation of elite scores with reversed sorted indexes.
- The same harness passed initial-score checks in both directions using normal population initialization, and controller checks showing slope 0 yields uniform rates while slope 1 changes rates across ranks.
- Local validation artifacts are under ignored `build/review-probes/` (`fixes.c`, `run-fixes.cmd`) and `build/review-fixes/` (`build.log`). They are not durable checked-in tests; use the recorded cases when adding regression coverage. The CMocka suite, asynchronous synchronization, complete GA behavior, and performance benchmarks were not run in this change.
- `git diff --check` passed.

## O3/O11 implementation update (2026-10-07)

This update supersedes the earlier design-only discussion and historical operator
counterexamples above. The approved implementation is complete for finite scores
and the documented valid operator parameters; broad validation remains O13.

- Added worker-owned `normalized_result_set`, `duplicate_flags`, `reseed_indexes`
  and `reseed_count` to the packed gene pool. Task initialization resets these
  buffers. Canonical/normalized/weight arrays retain physical individual order.
- Mandatory `process_normalize` copies canonical scores after ascending indexed
  sorting. Exact -DBL_MAX/+DBL_MAX map to 0/1 and are excluded from ordinary
  extrema, as explicitly requested. This also assigns sentinel treatment to a
  legitimate score equal to either exact endpoint. Equal ordinary scores map to
  1; opposite-sign ranges use half-sized differences when needed. Other finite
  score distinctions may still round together. Genes and canonical scores remain
  unchanged throughout normalization/flattening/selection.
- Retained adjacent equal-score SIMD chromosome comparisons and the existing
  worst-slot reseed policy `max(duplicates, reseed_bottom_N)`, capped to non-elites.
  Fixed its eight-bit equality-mask comparison (previously compared to a 64-bit
  all-ones constant). Duplicate flags replace `nextafter` score perturbations;
  there is no second sort. This does not introduce exhaustive duplicate search
  or change to reseeding each detected duplicate position. Those policies remain
  discussion items. Equal scores can represent different genes and normalization
  precedes reseeding, so equal-score handling is required.
- All six flatteners now read normalized scores. None/normalized copy u; linear
  uses safely scaled nonnegative affine weights; exp/log/sigmoid use bounded
  monotonic formulas. Alpha is nonnegative slope/gain/curvature; beta is the
  linear nonnegative baseline or sigmoid center in [0,1], unused for exp/log.
  Log uses its linear limit for alpha <= 1e-8. Invalid coefficients are not
  automatically corrected and remain outside the documented contract.
- Repaired roulette cumulative boundaries, overflow handling and zero-total
  fallback; tournament uses normalized comparisons with canonical tie resolution
  and random genuine ties; rank keeps worst=0/best=N-1, fixes weights/mapping,
  and shares rank mass among equal-score candidates. Rank-space implements
  `P(i)=(1-lambda)R_i/sum(R)+lambda D_i/sum(D)`, with each zero-total component
  uniform. D is mean squared centroid distance in normalized uint32 chromosome
  coordinates. Other phenotype/categorical metrics remain TODO.
- Rank caches are prepared only for rank methods and invalidated on dimension
  or relevant parameter changes. Tie adjustments are per population. Diversity
  buffers are reusable, reset before use, and fully freed with pointers cleared.
- Existing selector IDs 0..4 remain. ID 4 is repaired mixed Boltzmann; appended
  IDs 5/6/7 are pairwise logistic, strict Boltzmann and relaxed Boltzmann. The
  user explicitly confirmed exposing all four. Stable logistic probabilities
  use anti-acceptance for B/C and acceptance for A/winner. Class threshold defaults
  to zero in normalized units; bounded searches try max(1,N/10) draws and retain
  the last draw when no separated class is found, including tied populations.
  This is the paper's bounded-fallback approach on normalized fitness; comparative
  optimization performance and original-unit equilibrium guarantees are unproven.
- Deferred explicitly: reporting snapshots/adaptation timing changes (Q4/O9),
  the broader operator-parameter validation sweep (O13), and broader configuration
  exports/reproducibility metadata. Current validation is basic structural and
  execution-setting validation, so no partial coefficient-validation sweep was
  added. Requirements are recorded in docs/source/todo.md and operator contracts.
  Logger/dump_config code was not changed. O14 sorter tails and O15 automatic
  seeds remain separate open issues; the pipeline regression uses size 64 and
  explicit deterministic seeds.
- Public gene-pool/selection structs changed; clients must rebuild. A new
  selection_boltzmann_threshold field defaults to 0 through default_config.

Validation: MSVC 19.51 x64 Debug complete build passed. All eight CTest tests
passed with the existing CMocka DLL directory added to the test process PATH.
New test_fitness coverage includes sentinel and extreme arithmetic, all flatteners,
allocation/reset isolation, dedupe, selector frequency checks, cache resizing,
and three population iterations. New test_selection_boundaries drives the actual
selector with scripted RNG values for exact probability boundaries, rank mapping,
ties, mixed/strict/relaxed/pairwise behavior, and strict class-retry distinctions.
The final additional class-retry test also passed after rebuilding. These are
correctness checks, not a benchmark of optimization quality. git diff --check passed.

Work is split into concise commits for storage/normalization, duplicate bookkeeping,
flatteners, selectors/workspaces, tests, and documentation. Pre-existing app and
scratch-note edits are preserved separately from these staged changes.

## O9/Q4/Q8 implementation and measurement update (2026-10-07)

Commit: `ebe1902` — `Fix reported objective units and completed-task statistics`.
This implements the small result/statistics fixes, not the broader observer API.

### Implemented and validated (F9)

- `report_task()` converts direction-adjusted internal scores to original
  objective units when producing periodic/final records and CSV/binary score
  columns. Records retain the effective direction from the evaluated task,
  since built-ins may override the initial configuration. This does not resolve
  O8's concurrent direction writes inside objective dispatch.
- `record_completed_result()` runs in the logger once per final task record.
  It uses Welford mean/M2 updates and exposes sample standard deviation with
  denominator N-1. Best selection respects minimize/maximize; equal results keep
  the first winning report. The display consumes finalized statistics directly.
- With no completed tasks, best/mean/stddev are NAN, best-result iteration is
  UINT32_MAX, elapsed time starts at zero, and direction is zero until the first
  completion. With one completed task, best/mean are defined and sample stddev
  is NAN. Logger shutdown avoids writing a nonexistent best record when empty.
- The returned scalar and `progress_t` describe final task results in original
  objective units. Internal operator scores are unchanged. Rebuild clients for
  task-result/queue layout changes and update consumers for minimization signs
  and finalized mean/stddev semantics. The disposable nested-GA scorer explicitly
  converts its scalar input back to historical ranking units; its formula was
  not redesigned.
- MSVC x64 Debug build passed. Eight selected CTest regressions passed:
  `test_benchmarks`, `test_benchmark_dispatch`, `test_runtime`, `test_results`,
  `test_fitness`, `test_selection_boundaries`, and both invalid fitness-setting
  tests. New `Tests/agentic/test_results.c` covers empty/single-result behavior,
  known sample variance, reversed arrival order, large offsets, both directions,
  ties, periodic/final CSV and binary score conversion, effective task direction,
  and unchanged internal scores. Two short constant-objective solves verify the
  public scalar and progress statistics with zero elitism and population 64.
  These checks do not validate integer candidate correspondence, elite retention,
  arbitrary population sizes, or full optimization quality. `git diff --check`
  passed. The existing app and scratch-note edits were excluded from the commit.

### Q4 clarification and confirmed elite defect (O16)

The existing logger queue already owns copied report data; delayed file writing
is not the cause. The relevant question is what `report_task()` copies after
variation. `pop_param_double` retains decoded evaluated inputs, whereas
`pop_param_bin` contains chromosomes modified by crossover/mutation/reseeding.
Thus ordinary evaluated double CSV rows can still match their stored scores;
integer CSV rows can pair new chromosomes with old scores. Binary output always
copies decoded doubles, even for integer objectives. Unevaluated candidates
still need explicit validity semantics.

The elite-copy concern was verified rather than changed speculatively. A probe
linked the real `process_crossover()` with 64 individuals, 16 genes, two elites,
complete crossover, and seed 12345. The last elite's active buffer held 1063 and
its alternate buffer held 9063. After crossover both held 9063, proving loss of
the active elite. The `memcpy_s` destination/source are reversed for preserving
active elites into the next buffer before the pointer swap. Crossover was left
unchanged during that investigation, then repaired by F10 below. Source and
build scripts are in ignored `build/results-audit/`; rerunning the original probe
after F10 now records successful preservation of 1063.

### Agreed priorities and proposed bookkeeping placement

The user prefers preserving algorithm logic while capturing data at the correct
time, and keeping bookkeeping outside per-individual hot loops. The following
placement is documented but not implemented:

- Count evaluations once after `process_fx()` completes: currently add
  `individuals - elitism`; have the evaluator return an actual count if its
  policy later changes. No atomic increment per objective invocation is needed.
- After sorting, before variation, compare the winner with task-local best-ever
  state. Record its discovery iteration and copy evaluated inputs only on strict
  improvement. Allocate storage once; cost is one comparison per generation and
  O(genes) copying only when the best improves. This also retains earlier better
  results when elitism is zero.
- Capture requested top-N evaluated inputs/scores at the same boundary only on
  export generations. Avoid full-population snapshots every iteration. Publish
  the saved best-ever candidate if stopping is detected later; keep final-report
  iteration distinct from discovery iteration.
- Record a termination enum where `check_convergence()` detects the stopping
  condition; define precedence if multiple conditions fire. Both conditions
  currently collapse into `convergence_reached`.
- Preserve current variation/adaptation order while separating capture time
  from report publication. Candidate return, discovery iteration, evaluation
  count, and termination reason remain open under O9/Q4/Q8.
- Mutation/adaptation export fields were only debugging aids. User requested a
  TODO to remove them from CSV/binary solution output, including headers, sizing,
  and configuration counts. Do not prioritize a separate timing/rank-index fix
  for those fields ahead of removal. The TODO is recorded; fields are still saved.

Further detail: [results follow-up](docs/source/todo.md),
[result contract](docs/source/data_model.md), and [test notes](docs/source/tests.md).

### Q4 alternative proposal: stable evaluated-input buffer (2026-10-07)

User proposal accepted as a plausible design for further consideration, not an
implementation decision. Keep mutable working chromosomes separate from a
double-valued evaluated-input buffer. Reporting reads the evaluated inputs and
their matching scores, even when working chromosomes have already changed.
This is an alternative to copying top-N candidates before variation, not an
additional mandatory population snapshot. It may reuse `pop_param_double`,
which already supplies this separation for double-valued objectives.

- During evaluation, retain the exact input associated with the resulting score
  at the same physical individual index. For double objectives this is the
  existing bounded decoding. For integer objectives retain the actual masked/
  zoned uint32_t inputs passed to the objective, represented numerically as
  doubles; do not apply double-mode bounds decoding to integer inputs. Every
  uint32_t value is exactly representable by the current binary64 double type.
  The integer objective itself continues to receive its integer representation.
- Crossover, mutation and reseeding change working chromosomes only. The
  evaluated-input buffer and scores stay paired until that slot is evaluated
  again. "Static" means stable between evaluations, not immutable for the entire
  run. Cached elites retain both their evaluated input and cached score.
- Workers must finish writing inputs and scores before publishing evaluation
  completion. The existing release/acquire completion barrier can then make
  both visible to the solver. Reporting still copies data into queue-owned
  buffers before the next evaluation can overwrite it; do not enqueue pointers
  into the reusable evaluated-input storage.
- Route CSV and binary candidate serialization through this evaluated buffer.
  Decide and document integer formatting/binary layout explicitly; double
  storage does not require changing the integer objective API or losing type
  metadata. Existing callbacks can mutate their input pointers, so preserving
  exact call inputs requires either a read-only callback contract or a separate
  capture before the call; this point remains undecided.
- Track or otherwise define evaluation validity: initial unevaluated slots must
  not be exported as real evaluated solutions. Reusing an old evaluated record
  after variation is intentional; it describes the last evaluation, not the
  newly generated working chromosome. Reset validity at task boundaries and
  distinguish evaluation generation from current report generation if needed.
- Cost tradeoff: reusing the double buffer can avoid another population-sized
  allocation/copy in double mode. Integer mode adds O(genes) conversion/storage
  per evaluated individual, on the evaluation path. Compare that cost with
  capturing only requested reports before variation, especially for cheap
  integer objectives. No performance advantage has been measured.
- This can resolve Q4 correspondence while retaining the current reporting and
  adaptation order. It does not itself retain best-ever candidates once a slot
  is reevaluated, or add discovery iteration, evaluation counts, stop reasons,
  or the wider Q8 observer interface. Those remain separate follow-ups.

No implementation or tests were changed for this proposal. The user will
consider how to implement it before proceeding.

## O16 elite preservation repair (2026-10-07)

Commit: `5b7c01b` — `Preserve current elites in the next crossover buffer`.

- Implemented the user-approved source/destination reversal in the elite
  `memcpy_s` in `src/Utility/crossover.c::process_crossover()`. Current elites
  are copied to `pop_param_bin_cross_buffer` before the existing pointer swap.
  Crossover generation and operator/adaptation order remain unchanged.
- Added `Tests/agentic/test_crossover_elitism.c`, linked against the real library
  and registered in CTest's agentic group. It verifies every chromosome word,
  distinct active/alternate buffers, identity/permuted rank mappings, three
  successive buffer swaps, and elite counts 0, 1, 2, 3, and 64 for population 64
  with 16 genes. Identical selected parents give an exact expected child as well
  as a preservation check. Odd non-elite counts exercise the final child pair
  overlapping an elite slot that must then be restored.
- Verified failure before the fix: elites=2, generation=0, rank=62, slot=62.
  After the fix the regression passes. The original focused probe now reports
  active elite=1063 and old-buffer elite=1063, matching the expected 1063.
- MSVC x64 Debug build passed. The new regression and eight additional selected
  CTest tests passed: benchmarks, benchmark dispatch, runtime, results, fitness,
  selection boundaries, and both invalid fitness-setting cases. This verifies
  the shared elite-copy path using complete crossover at the tested dimensions;
  it is not broad validation of every crossover method or arbitrary sizes.
- Q4 integer candidate serialization, O14 sorter boundaries, and the remaining
  O9/Q8 result metadata are outside this repair. Existing app and scratch-note
  changes remain separate from the implementation commit.

## Keeping this file useful across sessions

- Keep stable IDs when moving items between open, decided, and fixed lists.
- For a fix, record the changed function, behavioral scope, checks run, and remaining limitations. Do not mark a broader issue fixed because one part was addressed.
- Treat design proposals as proposals until implemented or explicitly decided.
- Before editing, inspect the current branch and working-tree changes; this review worktree started detached at the `C-code` revision.
- Link detailed operator work to [the existing TODO list](docs/source/todo.md) and [test notes](docs/source/tests.md), and reconcile stale entries when their implementations change.
