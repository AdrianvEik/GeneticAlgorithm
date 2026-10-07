# GA project working notes

Last updated: 2026-10-07 (branch handoff; validation below was performed on 2026-10-03).

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
- **Configuration ownership is separate:** a separate gene pool does not imply separate storage for pointers inside `task->config_ga`; see O1 below.
- **Cascade status:** the current `optimize_fx_ga()`, scalers, and `weigh_result()` are a disposable WIP proof of concept. The user intends to replace them. Do not treat improving that scoring/decoding scheme as the current objective.

## Fixed in the current changes

- [x] **F4 — Unused crossover setting removed.** Removed `crossover_stepsize` from `crossover_param_t`; no consumers were found. Rebuild clients because the struct layout changed.
- [x] **F5 — Inline defaults and verified combinations.** `task_size_fx = 0`, `thread_count_fx = 0`, and `write_bin = 0` are explicit defaults. Both fitness settings must be zero for inline evaluation or both positive for queued evaluation. `verify_input_parameters()` rejects each mixed combination with a specific diagnostic and exit 250 before allocating queues or starting threads. Per user direction, settings are never silently normalized. Disabled queues allocate no resources; solver/logger threads still exist. Updated the WIP inner GA to explicitly request zero fitness workers alongside its zero batch size, preserving its intended inline behavior.
- [x] **F6 — Stable fixed-base task RNG streams (O5).** `process_task_thread()` reseeds before each real task through `seed_rand_task_threadlocal(base, task_id)`. For nonzero base seeds, unsigned addition wraps modulo 2^32 and derived zero directly initializes SFMT without requesting entropy. A base seed of zero retains automatic seeding, now per task. Verified replay after other tasks/random draws, execution on another worker, and wraparound against direct SFMT initialization. Full GA repeatability still depends on O1/O2 and objective behavior.

- [x] **F1 — Fitness batch boundary.** In `src/Function/Function.c::process_fx()`, clamp an inclusive batch end that reaches or exceeds `N - E` to `N - E - 1`. This excludes the first elite and avoids that one-past-population endpoint when elitism is zero. The subtraction is inside the non-empty scheduling loop. Completion synchronization remains O2.
- [x] **F2 — Initial score sign.** In `src/Utility/pop.c::fill_pop()`, replace the minimize/maximize sentinel conditional with `-DBL_MAX` for both modes. These are internal fitness scores, already in the maximization convention. This fixes the sentinel sign; generation-zero evaluation remains O3.
- [x] **F3 — Mutation slope arithmetic.** In `src/Optimisation/Optimizer.c::compute_mutation_rate()`, cast both `i` and `individuals` to `double` before division. Previously every ratio in the loop was zero. Retain the existing `i / N` definition; choosing `i / (N - 1)` would be a separate policy decision.

## Findings and remaining work

- [ ] **O1 — Shared mutable mutation rates.** `default_config()` in `src/Helper/Struct.c` allocates `mutation_param.mutation_rate` once. `init_task()` in `src/Multiprocessing/mp_solver_th.c` does `task->config_ga = config_ga`, copying the array pointer. `compute_mutation_rate()` writes through that pointer. Thus solver tasks from one configuration share the array: concurrent solvers can race, and successive tasks can inherit adaptations. Separate fitness workers are not the source of these writes. Proposal to discuss: give each task an initialized mutation-rate array and release it in `free_task()`, or move adaptive rates into explicitly owned solver state that resets for each task. A deep copy of every configuration field is unnecessary if immutable bounds remain borrowed with a defined lifetime. **No ownership changes have been made.**
- [ ] **O2 — Fitness completion protocol.** `process_fx()` resets only the first `fx_ready` element of each batch, while `wait_fx_tasks_finish()` polls every non-elite element. Correction to the earlier note: `init_gene_pool()` zeroes the entire allocation, including these flags. On subsequent evaluations, the other flags retain stale completion values. Workers write and the solver reads plain integers without synchronization. F1 fixes only the endpoint. See the concrete interleaving and proposed completion protocol below. Built-in evaluations also concurrently write the task's direction.
- [ ] **O3 — Full finite fitness range.** The user intends every flattener and selector to tolerate scores from `-DBL_MAX` through `DBL_MAX`, while retaining partial initial evaluation. Audit below identifies numeric failures, reversed ordering, invalid probability distributions, and incorrect rank mapping. No flattener or selector was changed. Safe sentinel ordering alone does not establish safe downstream arithmetic; operators must accept signed finite fitness and construct valid selection weights where required.
- [ ] **O4 — Remaining internal initialization audit.** All four configuration-default omissions are resolved by F4/F5: three fields now default to zero and unused `crossover_stepsize` is removed. The separate internal initialization inventory below remains open (CSV sizing, progress metadata, integer zone masks).
- [x] **O5 — Fixed-base seeds now belong to tasks (F6).** Real tasks reseed their solver TLS RNG before population initialization using `(base_seed + task_id) mod 2^32`. Derived zero is a deterministic SFMT seed. Base zero still requests automatic seeding per task; persisting automatic seeds is deferred. This fixes RNG stream assignment, not O1/O2 races or random external objectives.
- [ ] **O6 — Inline fitness is not a fully serial solve.** Both fitness settings zero disable fitness workers and their queue (F5); mixed zero/positive settings are invalid. Solver and logging threads still run. Preserve the distinction between an inline CPU evaluation backend and a fully serial GA implementation; discuss whether the latter is needed.
- [ ] **O7 — Worker polling overhead.** Queue operations use `Sleep(1000)`, evaluation completion uses `Sleep(10)`, and logging uses `Sleep(500)`. These delays can dominate cheap runs. Separate algorithm quality measurements from scheduling overhead; consider condition/event-based waits and reusable workers.
- [ ] **O8 — Objective dispatch is coupled to experiments.** Benchmark functions, GA tuning, decoding, scoring, and dispatch live in `Function.c`. The callback lacks user context and returns no evaluation status. Wheeler's Ridge has a method constant and implementation but no dispatch branch. Built-ins also override the direction during evaluation. Resolve objective contracts before expanding the API.
- [ ] **O9 — Result/statistics contract.** The public return currently reflects internal ranking scores; `progress_t` cannot return a best-candidate vector or evaluation count/stop reason. `average_result` is a sum, and `result_standard_deviation` is an accumulated quantity rather than a computed standard deviation. Audit the variance calculation before using it as an experimental metric. `best_result_iteration` records the winning task's final report generation, not necessarily when that value first appeared.
- [ ] **O10 — Cleanup across repeated runs.** `start_threads()` allocates a worker-parameter array through a local pointer that is not returned for cleanup. `Genetic_Algorithm()` does not call `free_task_result_queue()`. Queue cleanup also needs an ownership audit for allocated locks. Repeated/cascading runs need bounded memory use and complete teardown.
- [ ] **O11 — Selection assumptions.** Raw canonical scores can be negative, but roulette cumulative weights require a valid nonnegative distribution. Rank selection samples ranks without mapping them through `sorted_indexes`. Existing `docs/source/todo.md` also flags flattening monotonicity and Boltzmann acceptance issues. Validate these before treating all selectable operators as equivalent supported options.
- [ ] **O12 — Domain/task generation.** Integer evaluation applies `zone_mask` and `zone_id`, but the non-zoned task path does not initialize those allocated arrays. Integer splitting can shift by 32 for an unsplit dimension. Audit masks, shifts, actual generated task counts, and complete domain coverage before comparing partitioning against restarts.
- [ ] **O13 — Public error handling and validation.** Most failures call `exit()`, terminating an entire experiment sweep. Input validation covers only a few fields. Define behavior for invalid bounds, unsupported dimensions/population sizes, callback failures, NaN/infinity, and no valid candidates. Preserve failing cases as explicit outcomes in experiments.

## Questions and proposals requiring more reasoning

- [ ] **Q1 — Ownership choice for O1.** The user proposed moving mutable mutation rates into TLS alongside the existing mutation scratch buffer. This isolates current solver workers, but requires resetting values for every task and does not automatically support nested solves on the same thread. Recommendation to discuss: keep immutable starting settings in configuration, store mutable rates in an explicit run-state object, and optionally use TLS to point to active state. A TLS-owned array is an incremental alternative: allocate by population size (existing mutation scratch is gene-sized), reset before each task's `fill_pop()`, and free/reset pointers on worker teardown. No ownership change has been implemented.
- [ ] **Q2 — Evaluation semantics.** Retain intentional partial initial evaluation and make operators robust to the full finite fitness range (O3). Decide separately whether unevaluated sentinels need explicit metadata to distinguish them from legitimate worst-case objective values. Establish elite reuse for noisy objectives and behavior on evaluation failure. Use a generation barrier before selection/adaptation.
- [ ] **Q3 — Mutation controller behavior.** After F3, inspect response curves, denominator-zero cases, min/max limits, rank direction, and whether fixed mutation should be available as a baseline. The casts do not validate the whole controller.
- [ ] **Q4 — Measurement timing.** `process_pop()` evaluates and then changes chromosomes before `report_task()` runs. Audit that reported candidates still correspond to their reported scores, especially for multiple exported individuals or zero elitism. This is also flagged in `docs/source/todo.md`.
- [ ] **Q5 — SIMD and small sizes.** Establish supported population/gene sizes, alignment/tail handling, and CPU instruction requirements. The code forces AVX feature macros; build presets alone do not establish portability. Include a scalar reference or explicitly validate supported limits.
- [ ] **Q6 — Search and execution controls.** Separate independent searches, evaluation workers, evaluation batch size, restarts, and explicit subdomains. Define nested parallelism and worker budgets before reusing pools for cascades.
- [ ] **Q7 — Future objective API.** Consider typed read-only callbacks with user context, evaluation status, and a thread-safety/lifetime contract. Distinguish raw `uint32_t` chromosomes from bounded integer decision variables. Move benchmark objectives and tuning code into examples/experiments.
- [ ] **Q8 — Future results/observers.** Return original objective values, best candidates, evaluation counts, timing, termination reasons, seeds, and effective configuration. Observe evaluated populations before variation and record actual adaptive settings; let logging consume observations.
- [ ] **Q9 — New cascade design.** Decode named integer/real/categorical parameters with explicit constraints. Evaluate inner configurations over specified problems and seeds under fair budgets. Start with normalized final error under a fixed evaluation budget; report success, time, and variability separately. Validate selected configurations on held-out seeds/problems.

## Discussion details: O2–O5

### O2: why a batch can appear complete early

After an evaluation, flags for a four-individual batch are `[1, 1, 1, 1]`. Next generation resets only its first flag: `[0, 1, 1, 1]`. The worker finishes the first individual and writes its flag, producing `[1, 1, 1, 1]` while the other three are still being evaluated. Once other batches also appear complete, the solver can sort, select, mutate, or release task data that workers are still using. Sorted indexes themselves can change underneath the worker's loop.

There is also a separate synchronization problem: ordinary integer flags do not publish result writes safely between threads. The queue mutex protects enqueue/dequeue operations, not subsequent evaluation completion. `Sleep()` or `volatile` would not establish the missing synchronization.

Proposed protocol (not implemented): before publishing any jobs for a generation, initialize a per-population pending-batch count to the full job count. Each worker writes all results for its batch, then decrements that count under a completion mutex and signals a condition/event when it reaches zero. The solver waits on the same protected predicate in a loop before using results. Initialize the whole count up front so early workers cannot announce completion while jobs are still being submitted. An atomic per-individual protocol is another option, but would require resetting every active flag before submission and acquire/release publication semantics.

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
- `progress_t.best_result_iteration`: not assigned in `init_task_result_queue()`; assigned only when a completed task improves the best score. Define the value when no valid improvement is reported.
- `progress_t.elapsed_time`: not assigned by that initializer, but the logger assigns it before normal display and again on termination; this is staged initialization rather than an established bad read.
- `task_param_t.zone_mask[]` and `zone_id[]`: allocated by `init_task()` but not filled in the non-zoned path; integer objective evaluation subsequently reads them (O12).
- `fx_task_param_t.task_id`: never assigned for fitness jobs; no consumer found. `thread_param_t.status` is first assigned on task completion; no reader found. These are unused/incomplete metadata rather than demonstrated evaluation failures.
- Some fields are intentionally initialized later: solver task IDs by `add_task()`, iteration numbers by the solver loop, console message count by `Genetic_Algorithm()`, and file handles only for enabled outputs. Do not label every such field a bug.
- `fx_ready[]` is initially zeroed; the unresolved problem is reset per evaluation plus synchronization (O2).

### O5: stable task seeds

The user proposed `base_seed + sequential_task_number`. Implemented in F6: the single producer assigns `task_id` monotonically in `add_task()` before publishing each task. The solver reseeds its TLS RNG after dequeuing each real task and before `fill_pop()`.

Previously seeding happened once per worker, so tasks inherited whichever worker stream scheduling assigned them. The new fixed-base mapping gives the same logical task the same initial stream independently of prior draws or worker assignment. Task order and configuration still define task identity; changing the partition plan changes which problem a task ID represents.

Nonzero base seeds use addition modulo 2^32, including zero as a deterministic derived SFMT seed. Base zero still requests entropy on each task; resolving and persisting a single automatic run seed is deferred. Distinct task IDs across the 32-bit range give distinct seed bit patterns for a fixed base, but statistical independence is not established. This change does not remove O1/O2 races or control stochastic external objectives.

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

- F5 follow-up: replaced silent normalization with verifier errors as requested. MSVC Debug library rebuild and focused runtime checks passed. Both mixed fitness combinations were executed as separate processes and checked through `Tests/expect_runtime_error.cmake`: exact exit code 250 plus the corresponding diagnostic. Both-zero and both-positive settings pass validation. These two error cases are registered with CTest; the scripts were run directly in this validation. Full solver/CMocka tests were not run. The earlier mixed-setting queue test was removed because those settings are now invalid.

- F4/F5/F6: MSVC Debug library rebuild succeeded with existing warnings. Added durable `Tests/test_runtime.c` and registered `test_runtime` with CTest. Compiled and ran that test directly against the library: default zeros, disabled queue initialization/cleanup for zero settings, normal queue roundtrip, task-stream replay after other draws, execution on another worker, and wrapped seeds all passed. The full CMocka/CTest suite and complete solver were not run; the CTest registration was not exercised through a fresh test-enabled configure.
- O3 audit: 30 actual flattener evaluations (six methods by five inputs) plus deterministic selection probes produced the counterexamples recorded in the operator tables. Probes are local ignored artifacts under `build/review-probes/` (`flatten-audit.c`, `selection-audit.c`). Rank-space findings were established by source inspection. These investigations intentionally expose existing failures, not passing regression tests for repaired operators.
- Dispatch probe: actual queue implementation, three idle-worker samples using Windows performance counters, recorded above. Local artifact `build/review-probes/dispatch-audit.c`; no objective cost or device speedup inferred from this experiment.

- Earlier review: source inspection plus a standalone C arithmetic probe. No full solver benchmarks or end-to-end pass was claimed.
- Current F1/F2/F3 changes: MSVC 19.44 x64 Debug build of the complete `ga` static library succeeded (`BUILD_TESTING=OFF`, `BUILD_GA_EXE=OFF`). Existing compiler warnings remain. The default CMake on PATH was too old for the VS 2022 generator, so validation used Visual Studio's bundled CMake 3.31.6. MSBuild required access outside the sandbox for Windows SDK discovery.
- A local C harness tested the actual `Function.c` with a synchronous mock of queue submission, and linked `fill_pop()` and `adapt_param()` from the built library. Passed 88 evaluation cases: both directions; elitism 0, 3, 31, and 32 out of 32; and batch sizes 0 (inline), 1, 2, 3, 4, 8, 29, 30, 31, 32, and 64. Checked exact evaluation counts and preservation of elite scores with reversed sorted indexes.
- The same harness passed initial-score checks in both directions using normal population initialization, and controller checks showing slope 0 yields uniform rates while slope 1 changes rates across ranks.
- Local validation artifacts are under ignored `build/review-probes/` (`fixes.c`, `run-fixes.cmd`) and `build/review-fixes/` (`build.log`). They are not durable checked-in tests; use the recorded cases when adding regression coverage. The CMocka suite, asynchronous synchronization, complete GA behavior, and performance benchmarks were not run in this change.
- `git diff --check` passed.

## Keeping this file useful across sessions

- Keep stable IDs when moving items between open, decided, and fixed lists.
- For a fix, record the changed function, behavioral scope, checks run, and remaining limitations. Do not mark a broader issue fixed because one part was addressed.
- Treat design proposals as proposals until implemented or explicitly decided.
- Before editing, inspect the current branch and working-tree changes; this review worktree started detached at the `C-code` revision.
- Link detailed operator work to [the existing TODO list](docs/source/todo.md) and [test notes](docs/source/tests.md), and reconcile stale entries when their implementations change.
