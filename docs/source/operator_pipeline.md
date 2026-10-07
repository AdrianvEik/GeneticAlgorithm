# Operator Pipeline

The operator pipeline is centered on {c:func}`process_pop`, declared in
[`src/Utility/process.h`](../../src/Utility/process.h). One call represents one
genetic iteration for a solver task.

## Stage Order

The iteration connects functions in this order:

1. `process_fx` evaluates non-elites into canonical `pop_result_set`.
2. Indexed sorting fills ascending `sorted_indexes` without moving scores.
3. `dedupe_population` records adjacent equal-score/equal-chromosome matches
   in `duplicate_flags`. Its SIMD comparison uses the full eight-bit lane mask.
4. `process_normalize` copies canonical scores in physical order and normalizes
   that copy through the sorted view. Reserved finite endpoints map to 0/1 and
   are excluded from the interior range; equal interior scores map to 1.
   Opposite-sign ranges use half-sized differences when ordinary subtraction
   would overflow. Canonical values are preserved; no reevaluation is needed.
5. `process_flatten` maps normalized scores into nonnegative selection weights.
6. `process_selection` writes physical parent indexes into `selected_indexes`.
7. Crossover and mutation produce offspring.
8. Reseeding retains the existing worst-slot policy: replace
   `max(duplicate_count, reseed_bottom_N)` slots, capped to the non-elite count.
   Actual target slots are recorded separately in `reseed_indexes`. This does
   not switch to reseeding the detected duplicate positions themselves.

Dedupe no longer perturbs scores with `nextafter`, so its second sort is gone.
The adjacent scan is intentionally not an exhaustive equal-score-group search.

Adaptive updates happen outside this function through {c:func}`adapt_param`,
which reads the sorted best result and updates the gene pool's
`mutation_rate` array for the next iteration. {c:func}`init_mutation_rates`
restores the configured starting rate when a worker begins a new task.

## Population Seeding

Population ownership begins in
[`src/Utility/pop.h`](../../src/Utility/pop.h). {c:func}`init_gene_pool`
allocates one aligned memory region for all population buffers. {c:func}`fill_pop`
then selects either uniform or normal sampling based on
{c:type}`population_param_t`.

## Fitness Evaluation

Fitness evaluation is declared in
[`src/Function/Function.h`](../../src/Function/Function.h). {c:func}`process_fx`
either evaluates synchronously or splits the work into {c:type}`fx_task_param_t`
records. {c:func}`process_fx_set` evaluates a contiguous sorted-individual
range and stores signed results in `pop_result_set`.

## Genetic Operators

The operator dispatch headers document their public contracts:

- [`flatten.h`](../../src/Utility/flatten.h) owns fitness shaping before
  selection.
- [`selection.h`](../../src/Utility/selection.h) owns parent index selection.
- [`crossover.h`](../../src/Utility/crossover.h) owns recombination.
- [`mutation.h`](../../src/Utility/mutation.h) owns bit-level mutation.

The stage-specific configuration structs are documented in [Data Model](data_model.md).

## Fitness and selection contracts

All flatteners read `normalized_result_set` and write `flatten_result_set`:

| Method | Weight transformation | Coefficients |
| --- | --- | --- |
| None / normalized | `u` | Ignored; existing IDs retained |
| Linear | Proportional to `alpha*u + beta` | Finite nonnegative alpha and beta; coefficients scaled before addition |
| Exponential | `exp(alpha*(u-1))` | Finite nonnegative alpha; beta unused |
| Logarithmic | `log1p(alpha*u)/log1p(alpha)` | Finite nonnegative alpha; beta unused; linear limit used for alpha <= 1e-8 |
| Sigmoid | Stable logistic of `alpha*(u-beta)` | Finite alpha >= 0, center beta in [0,1] |

For valid parameters the outputs lie in [0,1]. No second minimum subtraction
is applied: that would change baseline weights. These ranges are documented
preconditions pending the project-wide validation sweep, not silently corrected
configuration settings.

Roulette scales by the largest weight before accumulating and samples a
half-open interval using strict cumulative boundaries. An all-zero weight set
becomes uniform. Tournament compares normalized scores, resolves rounding ties
using canonical fitness, and samples genuine ties fairly. Rank distributions
favor higher ascending ranks; equal canonical-score groups share their total
rank weight. Sampled ranks map through `sorted_indexes`.

Rank-space samples the mixture

    P(i) = (1-lambda) R_i/sum(R) + lambda D_i/sum(D), 0 <= lambda <= 1.

Each zero-total component becomes uniform. Diversity is mean squared distance
from the centroid of numeric uint32 chromosome coordinates divided by UINT32_MAX;
it is bounded, uses physical indexes, and does not cast or square signed ints.
Phenotype-aware, fixed-dimension and categorical metrics remain separate work.
Rank distributions are cached by size and their relevant parameter; equality
adjustments are rebuilt each population. Diversity buffers are reused and reset.
Only rank methods prepare rank distributions; other selectors do not depend on
unused rank parameters.

### Temperature-based selectors

Existing method IDs 0..4 are unchanged. ID 4 now repairs the paper's 50/50
strict/relaxed three-candidate method. New IDs are 5 (pairwise logistic),
6 (strict three-candidate), and 7 (relaxed three-candidate).

Three-candidate selection draws A uniformly. B must differ from A's normalized
fitness by more than `selection_boltzmann_threshold`. Strict C differs from both
A and B; relaxed C differs only from A. Each search makes at most
`max(1, N/10)` random attempts and falls back to its last draw if no separated
class was found. Repeats in this fallback are intentional and guarantee
termination for small or equal-score populations.

With normalized maximizing scores:

    P(B survives anti-acceptance against C) = sigmoid((u_C-u_B)/T)
    P(A wins against secondary winner W) = sigmoid((u_A-u_W)/T)
    P(A wins pairwise against B) = sigmoid((u_A-u_B)/T)

Stable logistic evaluation avoids exponential and quotient overflow for positive
finite T. Temperature and class separation are in normalized units; normalization
changes with the population, so original-objective equilibrium guarantees are not
claimed. Threshold zero is the explicit default; alternative class policies and
performance comparisons remain experiments.

Reporting still runs after variation, and mutation adaptation timing is unchanged.
Capturing evaluated candidate/score snapshots was explicitly deferred.
