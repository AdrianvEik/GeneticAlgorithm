/* Focused checks for defaults, disabled fitness queues, and task RNG streams. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/Helper/Struct.h"
#include "../src/Helper/rng.h"
#include "../src/Multiprocessing/mp_solver_th.h"

enum { SAMPLE_COUNT = 16 };

typedef struct {
    uint32_t base;
    uint32_t id;
    uint32_t sample[SAMPLE_COUNT];
} sample_task_t;

static void sample_task(sample_task_t *task) {
    seed_rand_task_threadlocal(task->base, task->id);
    for (unsigned i = 0; i < SAMPLE_COUNT; ++i) task->sample[i] = gen_mt_rand();
}

static THREAD_FUNC(sample_worker) {
    sample_task(arg);
    THREAD_NULL_RETURN;
}

static void check_defaults_and_inline_queue(void) {
    runtime_param_t runtime = default_runtime_param();
    assert(runtime.task_size_fx == 0);
    assert(runtime.thread_count_fx == 0);
    assert(runtime.logging_param.write_bin == 0);

    config_ga_t config = {0};
    config.fx_param.fx_data_type = fx_data_type_double;
    verify_input_parameters(config, runtime);

    fx_task_queue_t inline_queue;
    memset(&inline_queue, 0xA5, sizeof(inline_queue));
    init_fx_task_queue(&inline_queue, 0, 0, 0);
    assert(inline_queue.task_size_fx == 0);
    assert(inline_queue.queue_size == 0);
    assert(inline_queue.fx_task_list == NULL);
    assert(inline_queue.thread_id == NULL);
    assert(inline_queue.lock == NULL);
    free_fx_task_queue(&inline_queue);

    runtime.thread_count_fx = 2;
    runtime.task_size_fx = 4;
    verify_input_parameters(config, runtime);
    fx_task_queue_t queue;
    init_fx_task_queue(&queue, 8, 2, 4);
    assert(queue.task_size_fx == 4 && queue.queue_size == 8);
    assert(queue.fx_task_list && queue.thread_id && queue.lock);
    fx_task_param_t submitted = {0}, received;
    submitted.task_id = 17;
    submitted.individual_min = 2;
    submitted.individual_max = 5;
    add_fx_task(&queue, submitted);
    get_fx_task(&queue, &received);
    assert(received.task_id == 17 && received.individual_min == 2 && received.individual_max == 5);
    free_fx_task_queue(&queue);
}

static void check_task_streams(void) {
    sample_task_t first = {123, 0}, second = {123, 7}, replay = {123, 0};
    sample_task(&first);
    sample_task(&second);
    assert(memcmp(first.sample, second.sample, sizeof(first.sample)) != 0);
    for (unsigned i = 0; i < 101; ++i) (void)gen_mt_rand();
    sample_task(&replay);
    assert(memcmp(first.sample, replay.sample, sizeof(first.sample)) == 0);

    thread_t worker;
    replay.id = second.id;
    assert(thread_create(&worker, sample_worker, &replay) == 0);
    assert(thread_join(worker) == 0);
    assert(memcmp(second.sample, replay.sample, sizeof(second.sample)) == 0);

    /* Compare the task stream directly against SFMT for ordinary and wrapped seeds. */
    const uint32_t bases[] = {123, UINT32_MAX, UINT32_MAX};
    const uint32_t ids[] = {7, 1, 2};
    for (unsigned c = 0; c < 3; ++c) {
        sfmt_t reference;
        sfmt_init_gen_rand(&reference, (uint32_t)(bases[c] + ids[c]));
        seed_rand_task_threadlocal(bases[c], ids[c]);
        for (unsigned i = 0; i < SAMPLE_COUNT; ++i)
            assert(gen_mt_rand() == sfmt_genrand_uint32(&reference));
    }
}

int main(int argc, char **argv) {
    if (argc == 2) {
        runtime_param_t runtime = default_runtime_param();
        config_ga_t config = {0};
        config.fx_param.fx_data_type = fx_data_type_double;
        if (strcmp(argv[1], "--invalid-fx-no-workers") == 0) runtime.task_size_fx = 8;
        else if (strcmp(argv[1], "--invalid-fx-no-batch") == 0) runtime.thread_count_fx = 2;
        else return 2;
        verify_input_parameters(config, runtime);
        /* The error test expects exit 250 and a diagnostic from the verifier. */
        return 0;
    }
    check_defaults_and_inline_queue();
    check_task_streams();
    puts("PASS: runtime defaults, inline queue lifecycle, queued roundtrip, task seed replay/order/worker/wraparound.");
    return 0;
}
