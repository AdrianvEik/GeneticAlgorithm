# Operator Pipeline

The operator pipeline is centered on {c:func}`process_pop`, declared in
[`src/Utility/process.h`](../../src/Utility/process.h). One call represents one
genetic iteration for a solver task.

## Stage Order

The iteration connects functions in this order:

1. {c:func}`process_fx` evaluates non-elite individuals and fills
   `pop_result_set`.
2. The implementation sorts results and updates `sorted_indexes`.
3. Optional bottom-N reseeding refreshes weak individuals with
   {c:func}`fill_individual_uniform`.
4. {c:func}`process_flatten` transforms raw results into
   `flatten_result_set`.
5. {c:func}`process_selection` writes parent choices into `selected_indexes`.
6. {c:func}`process_crossover` recombines selected parents into the crossover
   buffer.
7. {c:func}`process_mutation` mutates sorted non-elite chromosomes in place.

Adaptive updates happen outside this function through {c:func}`adapt_param`,
which reads the sorted best result and updates `mutation_rate` for the next
iteration.

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
