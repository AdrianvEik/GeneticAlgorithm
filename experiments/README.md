# GA experiments from PowerShell

Run from this worktree. The C worker calls the production `Genetic_Algorithm`;
Python schedules isolated processes, validates their results and writes resumable
JSON files. No extra Python packages are required to gather results. Plots need
the packages in `requirements.txt`. Windows x64, MSVC C++ tools, CMake and Ninja
are required, as are the SIMD instructions already required by the library.

```powershell
Set-Location 'C:\Users\vanei\.codex\worktrees\d821\GeneticAlgortihm cm'

# Small real experiment: 256 runs, 16 repeats per combination, 500 generations.
.\experiments\Run-Benchmarks.ps1 -Profile smoke -Plots

# Inspect the full design without building or running it.
.\experiments\Run-Benchmarks.ps1 -DryRun

# Overnight: all combinations, 32 processes, 16 repeats, 10,000 generations.
# Stops scheduling after eight hours; lets in-flight runs finish and saves them.
.\experiments\Run-Benchmarks.ps1 -Hours 8 -Plots

# Resume with the same settings; completed valid runs are skipped.
.\experiments\Run-Benchmarks.ps1 -SkipBuild -Hours 8 -Plots

# Print interim results or recreate figures at any time.
python .\experiments\analyze.py .\build\experiments-full-results --plots
```

For missing plotting dependencies, install explicitly using your preferred Python:

```powershell
python -m pip install -r .\experiments\requirements.txt
```

Keep the machine awake and this PowerShell session open. With no `-Hours`, the
sweep runs until the entire matrix is complete. Ctrl+C stops new launches and
saves active runs before exit (at most the per-run timeout, default 900 seconds).
Failed or timed-out runs get an error file; the script exits unsuccessfully and
retries them on resume. Closing the process abruptly is also recoverable: only
valid atomically saved JSON files count as completed. One controller may write
to an output folder at a time. Disk space scales with trace density; increase
`--trace-interval` for very long experiments.

## Design and interpretation

The full design has **6 problems x 3 populations x 2 mutation x 8 selection x
6 flatten x 4 crossover x 16 seeds = 110,592 runs**, or 6,912 combinations.
There are 32 genes, encoded as 32-bit values, with populations 128, 256 and 512.
The smoke profile uses two problems, population 128, both mutations, roulette
and rank selection, exponential and none flattening, and uniform crossover.

| Problem | Bounds for each gene | Reference point for each gene |
|---|---|---|
| Styblinski–Tang | [-5, 5] | -2.903534027771178 |
| Ackley | [-32.768, 32.768] | 0 |
| Griewank | [-600, 600] | 0 |
| Levy | [-10, 10] | 1 |
| Rastrigin | [-5.12, 5.12] | 0 |
| Schwefel | [-500, 500] | 420.9687462275036 |

All objectives are minimized. Reference values are evaluated using this
repository's functions, including Schwefel's rounded constant, rather than
assuming its minimum is exactly zero. These domains and reference points follow
the benchmark definitions documented in `src/Function/Benchmarks.h`.
Langermann and Wheeler's Ridge are intrinsically two-dimensional in this repo
and are excluded from this 32-dimensional study. Wheeler's Ridge also lacks a
built-in dispatch branch. The nested GA objective is a different meta-optimization
task and is excluded.

Every combination uses repeat seeds `20261007 + repeat_index`, with the same seed
list across configurations. This supports paired comparisons; it does not mean
different methods consume matching random draws throughout a run. Execution
order is deterministically shuffled to reduce time-of-night ordering effects.
The default is 16 independent seeds, not 16 repeats of the identical seed.

Common fixed settings:

- One solver task and one solver thread per process, inline fitness, no zones.
  Up to 32 processes compute concurrently; each also has a mostly idle logger
  thread. This is an experiment throughput setup, not a GA thread-scaling test.
- Uniform initialization, 3 elites, reseed bottom 1 (existing duplicate policy
  may reseed more), mutation initial rate 3 and adaptive limits [1, 5].
- Mutation probability .5, slope 1; crossover probability .5; selection rank
  probability .2, temperature 10, tournament size 4, diversity fraction .5,
  rank distribution 0, Boltzmann threshold 0 (library defaults).
- Flatten alpha 4; beta .5 for sigmoid, .1 otherwise. Unused coefficients remain
  unused. These are explicit comparison settings, not individually tuned methods.
- Adaptive convergence threshold 1e-20 and moving window 200. Early convergence
  is disabled via a window equal to the full generation budget. The existing
  zero-based `iteration > max_iterations` check is translated with `budget - 2`.
  The worker verifies the exact observed count before accepting a result.

**Only roulette consumes flattened weights.** Other selectors use normalized
scores or ranks, so their solution outputs should match across flattening methods
with a fixed seed. Flatten `none` and `normalized` are equivalent implementations.
The requested full factorial retains these redundant cells as controls; do not
treat them as independent evidence that flattening helps those selectors.

Fixed generations give equal work budgets across methods within a population.
Different populations incur different evaluation counts. The implementation
evaluates `population - 3` individuals on each call, including the initial call;
its initially reserved elite slots are not evaluated on that first call. Thus
`evaluations = generations * (population - 3)` reflects actual production behavior.
For fair population comparisons at an equal evaluation budget, use separate
sweeps with adjusted generation counts or compare convergence at shared
evaluation counts; do not call the fixed-generation results equal-cost.

## Measurements and files

- `compute_seconds`: sum of high-resolution wall-clock intervals around
  `process_pop` and `adapt_param`. Includes fitness, sorting, selection, crossover,
  mutation and reseeding; excludes initialization, observers, reporting and
  shutdown. It includes OS scheduling delays under concurrent load. It is not
  CPU time or an isolated-core benchmark. The optional observer enables timing;
  ordinary runs with a NULL observer retain their original path.
- `generations_per_second`: actual completed calls divided by compute seconds.
- `wall_seconds`: full production GA call, including allocation, worker startup,
  queue polling, final logging and joins. Existing queues sleep for up to one
  second, so short solves can have much larger latency than compute time.
- `process_seconds`: controller-observed worker lifetime, also including process
  startup and JSON transfer. `sessions.jsonl` records overall sweep throughput;
  do not sum per-process wall times to estimate whole-sweep elapsed time.
- Objective and gap per gene at the final budget; sample SD uses `n-1` across
  seeds. A small SD alone does not imply proximity to the optimum.
- Target success: objective gap per gene <= 0.001. First observed hit generation
  and cumulative compute time are saved. Zero target generation means **not hit**;
  unsuccessful trials are censored, not zero-time successes. Summaries report hit
  rate and median time among successes only. These measurements do not stop runs.
- Sparse convergence samples `[completed_generations, compute_seconds, objective]`
  and all 32 final parameters, checked by reevaluating the saved solution.

`manifest.json` records the design, executable/controller hashes, CPU count, OS,
Python, Git revision and dirty status. `source.zip` preserves source and scripts;
`source.diff` captures tracked changes. A different build or design requires a new
output folder to avoid mixing incompatible data. Keep the matching executable
for resume. Outputs go under ignored `build/` by default.

`report/runs.csv` contains scalar per-run measurements; `report/summary.csv`
contains every observed combination, with mean, sample SD, median, success rate
and completeness. The shell and plots rank only combinations with all repeats.
Figures include final quality with SD, convergence with SD bands and evaluation
counts, quality versus generation speed, compute versus end-to-end latency, and
factor averages. PNG and PDF are both exported. Scores are compared within each
problem, not pooled across unrelated objective scales. Bands are descriptive SD,
not confidence intervals. Factor averages can hide interactions, especially in
partial sweeps; use the full per-combination tables for conclusions. Selecting
the winner from thousands of combinations is exploratory; confirm shortlisted
methods on additional held-out seeds before claiming superiority.

## Custom pilot and validation

```powershell
.\experiments\Build-Benchmark.ps1
python .\experiments\test_experiments.py --exe .\build\experiments-release\ga_bench.exe

# Representative full-budget pilot: six problems, all three populations,
# one operator combination, 16 seeds = 288 runs.
python .\experiments\run_sweep.py --exe .\build\experiments-release\ga_bench.exe `
  --output .\build\experiments-pilot-results --mutation 1 --selection 0 `
  --flatten 5 --crossover 2 --generations 10000
python .\experiments\analyze.py .\build\experiments-pilot-results --plots

# Individual worker call: problem population generations seed mutation selection flatten crossover trace_interval
.\build\experiments-release\ga_bench.exe rastrigin 128 1000 12345 1 0 5 2 100
```

Method IDs are listed in `run_sweep.py` and `manifest.json`. CLI filters accept
multiple IDs: for example `--selection 0 1 2 --crossover 0 2`. Use `--workers 1`
and a separate output directory for an isolated latency baseline. Runtime ETA is
estimated from completed runs during the sweep and changes with method mix;
the smoke test is not a reliable full-budget overnight duration prediction.
