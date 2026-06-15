#define gen_mt_rand __wrap_gen_mt_rand
#define gen_mt_rand512 __wrap_gen_mt_rand512

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "../src/Utility/crossover.c"
#include "../src/Helper/Struct.h"


uint32_t __wrap_gen_mt_rand();
uint32_t __wrap_gen_mt_rand() {
    return mock_type(uint32_t); // Return a fixed value for testing
}

__m512i __wrap_gen_mt_rand512();
__m512i __wrap_gen_mt_rand512() {
    const void* p = mock_ptr_type(const void*);

    /*
     * Use the unaligned load unless you are absolutely sure the pointed-to
     * object is 64-byte aligned.
     */
    return _mm512_loadu_si512(p);
}

typedef struct test_fixture_struct_def {
    int a;
    /* test-owned state */
} test_fixture_t;

static int setup(void** state)
{
    test_fixture_t* fixture = malloc(sizeof(test_fixture_t));
    if (fixture == NULL) {
        return -1;
    }

    /* initialize fixture */

    *state = fixture;
    return 0;
}

static int teardown(void** state)
{
    test_fixture_t* fixture = (test_fixture_t*)(*state);

    /* destroy fixture-owned objects */

    free(fixture);
    *state = NULL;
    return 0;
}

static void test_something(void** state)
{
    test_fixture_t* fixture = (test_fixture_t*)(*state);
    /* assertions */
    assert_true(fixture != NULL);
}

//static void single_point_crossover(uint32_t* parent1, uint32_t* parent2, uint32_t* child1, uint32_t* child2, uint32_t genes, uint32_t individual_mem_size) {

static void test_single_cross_over(void** state) {
    int split_point = 42 + 512; // Example split point
    
    will_return(__wrap_gen_mt_rand, split_point);

    uint32_t parent1[32];
    uint32_t parent2[32];

    for (int i = 0; i < 32; i++) {
        parent1[i] = 0x00000000; // All bits set to 1
        parent2[i] = 0xFFFFFFFF; // All bits set to 0
    }

    uint32_t child1[32];
    uint32_t child2[32];


    single_point_crossover(parent1, parent2, child1, child2, 2, sizeof(parent1));

    for (int i = 0; i < 32; i++) {
        printf("child1[%d] = 0x%08X, child2[%d] = 0x%08X\n", i, child1[i], i, child2[i]);
        if (i < split_point / 32) {
            assert_int_equal(child1[i], 0x00000000);
            assert_int_equal(child2[i], 0xFFFFFFFF);
        } else if (i > split_point / 32) {
            assert_int_equal(child1[i], 0xFFFFFFFF);
            assert_int_equal(child2[i], 0x00000000);
        } else {
            assert_int_equal(child1[i], 0x003FFFFFu);
            assert_int_equal(child2[i], 0xFFC00000u);
        }
    }
}

static void test_two_point_cross_over(void** state) {
    int split_point_1 = 42 + 512; // Example split point
    int split_point_2 = 84 + 1024; // Example split point

    will_return(__wrap_gen_mt_rand, split_point_1);
    will_return(__wrap_gen_mt_rand, split_point_2);

    uint32_t parent1[64];
    uint32_t parent2[64];

    for (int i = 0; i < 64; i++) {
        parent1[i] = 0x00000000; // All bits set to 1
        parent2[i] = 0xFFFFFFFF; // All bits set to 0
    }

    uint32_t child1[64];
    uint32_t child2[64];


    two_point_crossover(parent1, parent2, child1, child2, 2, sizeof(parent1));

    for (int i = 0; i < 64; i++) {
        printf("child1[%d] = 0x%08X, child2[%d] = 0x%08X\n", i, child1[i], i, child2[i]);
        if ((i < split_point_1 / 32) || (i > split_point_2 / 32)) {
            assert_int_equal(child1[i], 0x00000000);
            assert_int_equal(child2[i], 0xFFFFFFFF);
        }
        else if (i > split_point_1 / 32 && i < split_point_2 / 32) {
            assert_int_equal(child1[i], 0xFFFFFFFF);
            assert_int_equal(child2[i], 0x00000000);
        }
        else if (i == split_point_1 / 32) {
            assert_int_equal(child1[i], 0x003FFFFFu);
            assert_int_equal(child2[i], 0xFFC00000u);
        }
        else {
            assert_int_equal(child1[i], 0xFFFFF000u);
            assert_int_equal(child2[i], 0x00000FFFu);
        }
    }
}

static void test_uniform_cross_over(void** state) {

    uint64_t mock_rng_words[8] = {
        0x1111111111111111ull,
        0x2222222222222222ull,
        0x3333333333333333ull,
        0x4444444444444444ull,
        0x5555555555555555ull,
        0x6666666666666666ull,
        0x7777777777777777ull,
        0x8888888888888888ull,
    };

    will_return(__wrap_gen_mt_rand512, mock_rng_words);

    uint32_t parent1[16];
    uint32_t parent2[16];

    for (int i = 0; i < 16; i++) {
        parent1[i] = 0x00000000; // All bits set to 1
        parent2[i] = 0xFFFFFFFF; // All bits set to 0
    }

    uint32_t child1[16];
    uint32_t child2[16];

    uniform_crossover(parent1, parent2, child1, child2, 2, sizeof(parent1));

    uint32_t expected_child1[16] = {
        0x11111111, 0x11111111,
        0x22222222, 0x22222222,
        0x33333333, 0x33333333,
        0x44444444, 0x44444444,
        0x55555555, 0x55555555,
        0x66666666, 0x66666666,
        0x77777777, 0x77777777,
        0x88888888, 0x88888888
    };

    uint32_t expected_child2[16] = {
        0xEEEEEEEE, 0xEEEEEEEE,
        0xDDDDDDDD, 0xDDDDDDDD,
        0xCCCCCCCC, 0xCCCCCCCC,
        0xBBBBBBBB, 0xBBBBBBBB,
        0xAAAAAAAA, 0xAAAAAAAA,
        0x99999999, 0x99999999,
        0x88888888, 0x88888888,
        0x77777777, 0x77777777,
    };

    for (int i = 0; i < 16; i++) {
        printf("child1[%d] = 0x%08X, child2[%d] = 0x%08X\n", i, child1[i], i, child2[i]);
        assert_int_equal(child1[i], expected_child1[i]);
        assert_int_equal(child2[i], expected_child2[i]);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_single_cross_over, setup, teardown),
        cmocka_unit_test_setup_teardown(test_two_point_cross_over, setup, teardown),
        cmocka_unit_test_setup_teardown(test_uniform_cross_over, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}