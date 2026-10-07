# Tests

The repository uses CMocka operator tests and linked assertion-based runtime/fitness tests under `Tests/`. Test
registration happens in [`Tests/CmakeLists.txt`](../../Tests/CmakeLists.txt),
which is included from the top-level CMake build when `BUILD_TESTING` is
enabled.

## Test Organization

Handwritten crossover, mutation, and sorting tests remain directly under `Tests/`.
Agent-authored regression tests and their error-checking helper are in
[`Tests/agentic`](../../Tests/agentic), with their own CMake registration:

- [`test_runtime.c`](../../Tests/agentic/test_runtime.c)
- [`test_results.c`](../../Tests/agentic/test_results.c)
- [`test_fitness.c`](../../Tests/agentic/test_fitness.c)
- [`test_selection_boundaries.c`](../../Tests/agentic/test_selection_boundaries.c)
- [`expect_runtime_error.cmake`](../../Tests/agentic/expect_runtime_error.cmake)

CTest still runs all tests by default. Run only the agent-authored tests with
`ctest --test-dir <build-dir> -L agentic --output-on-failure`, or exclude them
with `-LE agentic`. Test names and behavior are unchanged.

## Existing Tests

### `test_crossover`

Source: [`Tests/test_crossover.c`](../../Tests/test_crossover.c)

This test file includes [`src/Utility/crossover.c`](../../src/Utility/crossover.c)
directly so it can exercise static crossover helpers with deterministic mocked
RNG output.

Covered behavior:

- `single_point_crossover` keeps parent bits before the mocked split point and
  swaps bits after it.
- `two_point_crossover` swaps only the mocked middle region between split
  points.
- `uniform_crossover` combines parents using a mocked AVX-512 mask and checks
  both children word by word.

### `test_mutation`

Source: [`Tests/test_mutation.c`](../../Tests/test_mutation.c)

This test file includes [`src/Utility/mutation.c`](../../src/Utility/mutation.c)
directly so it can exercise the static bitwise mutation helper with mocked RNG
output.

Covered behavior:

- `mutate_individual_bitwise` flips deterministic bits in selected memory
  blocks.
- Memory blocks without a mutation event are left unchanged.

## Running Tests

From a configured CMake build directory:

```powershell
cmake --build <build-dir>
ctest --test-dir <build-dir> --output-on-failure
```

The top-level CMake file fetches CMocka with `FetchContent`, so the first
configure/build needs network access unless CMocka is already available in the
build cache.

## Test Notes

`test_results` checks empty/single/completed result semantics, sample standard
deviation against a known dataset, reversed completion order, large offsets,
minimize/maximize best selection, tie handling, periodic/final CSV and binary
score conversion, effective task direction, and unchanged internal scores.
Two short constant-objective solves verify the public scalar and progress
statistics for both directions. These checks use zero elitism and a
sorter-compatible population; they do not validate elite preservation or
integer candidate correspondence.

`test_runtime` links the library and covers defaults, queue completion, task RNG
streams, and mutation ownership. `test_fitness` also links the library and covers
sentinel exclusion, opposite-extreme ranges, subnormals, equal scores, all six
flatteners, allocation/reset isolation, adjacent SIMD dedupe, statistical selector
probabilities, tied ranks, diversity mixtures, workspace resizing, and three
inline population iterations at a sorter-compatible size of 64. It does not claim
arbitrary-size sorter safety or complete end-to-end optimization quality.

`test_selection_boundaries` compiles the production selector with a scripted RNG.
It checks exact zero/maximum/cumulative draws, zero/overflowing weight totals,
physical rank mapping, canonical tie resolution, randomized true ties, both
mixed branches, strict-versus-relaxed class retries, fallback termination, and
anti-acceptance/acceptance directions. Assertions fail on stderr rather than a
Windows dialog. Performance comparisons, reporting, task generation, and broad
end-to-end experiments remain in [TODO](todo.md).

On the current Windows CMocka build, put `_deps/cmocka-build/src` from the build
directory on the test process PATH so CTest can locate `cmocka.dll`. All eight
registered tests passed with that environment after the fitness changes.
