# Tests

The repository currently uses CMocka tests under `Tests/`. Test
registration happens in [`Tests/CmakeLists.txt`](../../Tests/CmakeLists.txt),
which is included from the top-level CMake build when `BUILD_TESTING` is
enabled.

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

The current tests are focused unit tests for low-level genetic operators. They
do not yet exercise the full `ga` static library as a linked unit, the solver
thread queues, task generation, logging, or end-to-end optimization behavior.
Those gaps are tracked in [TODO](todo.md).
