#define gen_mt_rand __wrap_gen_mt_rand
#define gen_mt_rand512 __wrap_gen_mt_rand512

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "../src/Utility/mutation.c"
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

static void test_mutation(void** state) {
    int flip_bit_1 = 42; // Example split point
    int flip_bit_2 = 24; // Example split point

    will_return(__wrap_gen_mt_rand, 1);

    will_return(__wrap_gen_mt_rand, flip_bit_1);
    // 0001 0001 0001 0001 0001 0001 0001 0001
    // 0001 0001 0101 0001 0001 0001 0001 0001

    will_return(__wrap_gen_mt_rand, 0);

    will_return(__wrap_gen_mt_rand, 1);

    will_return(__wrap_gen_mt_rand, flip_bit_2);
    // 1001 1001 1001 1001 1001 1001 1001 1001
    // 1001 1001 1001 1001 1001 1000 1001 1001

    uint32_t individual[48] = {
        0x11111111, 0x11111111,
        0x22222222, 0x22222222,
        0x33333333, 0x33333333,
        0x44444444, 0x44444444,
        0x55555555, 0x55555555,
        0x66666666, 0x66666666,
        0x77777777, 0x77777777,
        0x88888888, 0x88888888,

        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,

        0x99999999, 0x99999999,
        0xAAAAAAAA, 0xAAAAAAAA,
        0xBBBBBBBB, 0xBBBBBBBB,
        0xCCCCCCCC, 0xCCCCCCCC,
        0xDDDDDDDD, 0xDDDDDDDD,
        0xEEEEEEEE, 0xEEEEEEEE,
        0xFFFFFFFF, 0xFFFFFFFF,
        0x00000000, 0x00000000

        
    }; // All bits set to 0

    uint32_t expected_result[48] = {
        0x11111111, 0x11311111,
        0x22222222, 0x22222222,
        0x33333333, 0x33333333,
        0x44444444, 0x44444444,
        0x55555555, 0x55555555,
        0x66666666, 0x66666666,
        0x77777777, 0x77777777,
        0x88888888, 0x88888888,

        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,

        0x99999919, 0x99999999,
        0xAAAAAAAA, 0xAAAAAAAA,
        0xBBBBBBBB, 0xBBBBBBBB,
        0xCCCCCCCC, 0xCCCCCCCC,
        0xDDDDDDDD, 0xDDDDDDDD,
        0xEEEEEEEE, 0xEEEEEEEE,
        0xFFFFFFFF, 0xFFFFFFFF,
        0x00000000, 0x00000000
    }; // All bits set to 1


    mutate_individual_bitwise(
        3, // Memory blocks (24 bytes / 8 bytes per block)
        1.0, // Mutation probability of 1 to ensure mutation occurs
        (__mAVXi*)individual
    );

    for (int i = 0; i < 48; i++) {
        printf("individual[%d] = 0x%08X\n", i, individual[i]);
        assert_int_equal(individual[i], expected_result[i]);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_mutation, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}