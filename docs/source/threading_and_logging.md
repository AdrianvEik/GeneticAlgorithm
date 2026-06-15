# Threading and Logging

The runtime uses bounded ring buffers to keep solver work, fitness subtasks,
result logging, and console display loosely connected. The relevant headers are
under `src/Multiprocessing/` and `src/Logger/`.

## Solver Task Queue

{c:type}`task_queue_t` carries {c:type}`task_param_t` records from
{c:func}`make_task_list` to solver threads. Use:

- {c:func}`init_task_queue` and {c:func}`free_task_queue` for queue lifetime.
- {c:func}`init_task` and {c:func}`free_task` for per-task bounds and masks.
- {c:func}`add_task` and {c:func}`get_task` for producer and consumer access.
- {c:func}`stop_task_solver_threads` to enqueue termination records and join
  solver workers.

Tasks are generated in [`mp_task_gen.c`](../../src/Multiprocessing/mp_task_gen.c).
When `zone_enable` is set, search bounds or integer masks are split per gene
before tasks enter the queue.

## Fitness Task Queue

{c:type}`fx_task_queue_t` carries {c:type}`fx_task_param_t` slices for parallel
fitness evaluation. {c:func}`process_fx` chooses between synchronous
evaluation and this queue based on `task_size_fx`.

The queue is managed with {c:func}`init_fx_task_queue`,
{c:func}`add_fx_task`, {c:func}`get_fx_task`, {c:func}`free_fx_task_queue`, and
{c:func}`stop_fx_task_threads`.

## Result Logging

Solver threads call {c:func}`report_task` to serialize best or top-N
individuals into {c:type}`task_result_t`. The records are pushed into
{c:type}`task_result_queue_t`, where the logging thread:

- writes CSV or binary data through {c:func}`write_file_buffer`;
- updates {c:type}`progress_t`;
- keeps a deep copy of the best final record with {c:func}`copy_task_result`;
- displays progress through {c:func}`display_progress`.

Configuration export is handled separately by {c:func}`write_config`, which
produces a JSON snapshot next to the configured log output.

## Console Messages

{c:type}`console_queue_t` carries formatted messages from solver threads to the
logging thread. Producers use {c:func}`con_printf`; the logger drains with
{c:func}`get_from_console_queue` and renders via {c:func}`display_console_message`.

## Portability Layer

[`mp_thread_wrapper.h`](../../src/Multiprocessing/mp_thread_wrapper.h) provides
the cross-platform thread and mutex names used by the queue layer:
`thread_t`, `thread_mutex_t`, {c:func}`thread_create`, {c:func}`thread_join`,
and the `thread_mutex_*` helpers.
