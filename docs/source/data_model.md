# Data Model

The shared data model is concentrated in
[`src/Helper/Struct.h`](../../src/Helper/Struct.h). Most functions take either
{c:type}`gene_pool_t`, {c:type}`config_ga_t`, {c:type}`runtime_param_t`, or a
stage-specific parameter struct.

## Gene Pool

{c:type}`gene_pool_t` is the central mutable state for one solver worker. It
contains:

- Binary chromosomes in `pop_param_bin`.
- A crossover staging buffer in `pop_param_bin_cross_buffer`.
- Decoded floating-point values in `pop_param_double`.
- Canonical, direction-adjusted measurements in `pop_result_set`.
- Mandatory normalized scores in `normalized_result_set`.
- Finite nonnegative selection weights in `flatten_result_set`.
- Physical-index duplicate markers and reseed targets in `duplicate_flags` and
  `reseed_indexes`, with active length `reseed_count`.
- Mutable per-rank mutation rates in `mutation_rate`.
- Sorted, selected, and temporary index buffers.
- `fx_ready` flags for parallel fitness subtasks.

Allocate it with {c:func}`init_gene_pool`, seed it with {c:func}`fill_pop`, and
release it with {c:func}`free_gene_pool`. The new score and bookkeeping arrays
share that worker-owned allocation and reset when a task fills its population.
All value arrays use physical individual indexes. `sorted_indexes` traverses
canonical fitness from worst (rank 0) to best (rank N-1); it does not reorder
those arrays. Normalization, flattening, selection, and duplicate marking do
not change canonical measurements or genes.

Exact `-DBL_MAX` and `DBL_MAX` values map to 0 and 1 and are excluded from the
ordinary-score extrema. This policy also applies to legitimate measurements
that equal those exact endpoints. Equal ordinary scores map to 1; equal genes
are not implied by equal scores. Dedupe only compares adjacent equal-score
candidates, and reseeding happens after selection and variation.

The added fields change public structure layouts; rebuild clients. The new
`selection_boltzmann_threshold` defaults to zero (unequal normalized scores
form different classes). Parameter validation and configuration-export changes
are deliberately deferred to the broader sweeps tracked in TODO.

## Config Structs

{c:type}`config_ga_t` groups all algorithm choices. The default is created by
{c:func}`default_config` and must be released with {c:func}`free_config_ga`
because it owns heap arrays for population bounds. Mutation configuration
contains a scalar starting rate; each worker's gene pool owns and resets the
mutable per-rank rates for every task.

{c:type}`runtime_param_t` controls dimensions and execution settings:
population size, gene count, elitism, bit width, solver task count, worker
counts, random seed, zone splitting, and logging settings.

## Fitness Data Types

{c:type}`fx_param_t` selects how the objective receives individuals:

- `fx_data_type_double` decodes each chromosome into `pop_param_double` using
  task bounds before calling the objective.
- `fx_data_type_int` applies integer zone masks and passes binary genes through
  the integer path.

User callbacks use {c:type}`fx_ptr_generic`. The implementation applies
`fx_optim_mode_minimize` or `fx_optim_mode_maximize` after the objective
returns so the rest of the pipeline can sort in one direction.

## Queue Records

The multiprocessing layer uses plain structs as queue records:

- {c:type}`task_param_t` describes one solver task and its zone.
- {c:type}`fx_task_param_t` describes one fitness-evaluation slice.
- {c:type}`task_result_t` holds serialized logging data.
- {c:type}`console_message_t` holds formatted console output.

Those records are passed through the queues described in
[Threading and Logging](threading_and_logging.md).
