
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "../src/Helper/sort.c"

typedef struct test_fixture_struct_def {
    double random_numbers[128];
    double random_numbers_with_duplicates[128];
    uint32_t random_numbers_index[128];
    uint32_t index_found[128];
    double temp_workspace[128];
    double test_array[128];
    uint32_t with_duplicates;
    /* test-owned state */
} test_fixture_t;

static int setup(void** state)
{
    test_fixture_t* fixture = malloc(sizeof(test_fixture_t));
    if (fixture == NULL) {
        return -1;
    }

    double random_numbers[128] = {
    0.8241, 0.1499, 0.4937, 0.9312, 0.6214, 0.0853, 0.7715, 0.4128,
    0.5098, 0.9842, 0.2356, 0.6741, 0.1194, 0.8502, 0.3649, 0.5891,
    0.7235, 0.0478, 0.9163, 0.5342, 0.2847, 0.7921, 0.4503, 0.6128,
    0.1984, 0.8837, 0.3412, 0.7495, 0.0128, 0.5694, 0.9501, 0.2743,
    0.6582, 0.1259, 0.8143, 0.4628, 0.9812, 0.3015, 0.7189, 0.5327,
    0.0894, 0.6341, 0.2215, 0.8673, 0.4051, 0.9932, 0.1654, 0.7502,
    0.5849, 0.3128, 0.9241, 0.0457, 0.6912, 0.2483, 0.8375, 0.1098,
    0.4781, 0.8924, 0.0563, 0.6219, 0.3842, 0.7714, 0.1925, 0.5487,
    0.9631, 0.2314, 0.8012, 0.4395, 0.6718, 0.1284, 0.8591, 0.3547,
    0.5812, 0.7412, 0.0985, 0.9142, 0.4125, 0.6834, 0.2014, 0.8841,
    0.3475, 0.7018, 0.1583, 0.5564, 0.9742, 0.2541, 0.8291, 0.4812,
    0.1128, 0.6429, 0.3854, 0.7915, 0.0634, 0.5218, 0.9412, 0.2874,
    0.7351, 0.1894, 0.8521, 0.4218, 0.6185, 0.0471, 0.8921, 0.3142,
    0.5614, 0.7812, 0.2195, 0.9654, 0.3582, 0.6931, 0.1248, 0.8412,
    0.4698, 0.0851, 0.7128, 0.2584, 0.9015, 0.3891, 0.6247, 0.1549,
    0.8714, 0.4021, 0.9981, 0.0354, 0.6783, 0.2891, 0.8124, 0.5029
    };

    memcpy(fixture->random_numbers, random_numbers, sizeof(random_numbers));

    double random_numbers_with_duplicates[128] = {
    0.5362, 0.8293, 0.5054, 0.5370, 0.6554, 0.6499, 0.9145, 0.7045,
    0.6981, 0.1635, 0.4589, 0.0007, 0.0066, 0.6409, 0.6185, 0.6357,
    0.2467, 0.2780, 0.6817, 0.8058, 0.3403, 0.5773, 0.5846, 0.0322,
    0.7207, 0.9730, 0.0251, 0.3702, 0.6817, 0.9366, 0.0910, 0.0799,
    0.9960, 0.8094, 0.1116, 0.0636, 0.8616, 0.3649, 0.2096, 0.1011,
    0.4537, 0.3366, 0.6274, 0.0116, 0.8763, 0.0928, 0.2187, 0.5291,
    0.6767, 0.2634, 0.3786, 0.0459, 0.2233, 0.2649, 0.8428, 0.6480,
    0.6400, 0.6400, 0.2649, 0.2669, 0.6846, 0.8977, 0.0322, 0.2467,
    0.8607, 0.1555, 0.2628, 0.5892, 0.4537, 0.3155, 0.9428, 0.9974,
    0.0472, 0.1989, 0.5892, 0.2096, 0.4348, 0.3994, 0.8921, 0.3795,
    0.5095, 0.6480, 0.2328, 0.6091, 0.9710, 0.0266, 0.9571, 0.2670,
    0.9537, 0.2280, 0.9894, 0.4222, 0.3147, 0.7920, 0.2205, 0.2678,
    0.0968, 0.5569, 0.2194, 0.5520, 0.8071, 0.7759, 0.8758, 0.2280,
    0.7364, 0.5362, 0.6499, 0.0870, 0.5614, 0.1097, 0.0299, 0.6394,
    0.5394, 0.6981, 0.6037, 0.2110, 0.3957, 0.2751, 0.2894, 0.4219,
    0.8474, 0.7297, 0.5449, 0.7291, 0.2291, 0.8058, 0.1712, 0.3816
    };

    memcpy(fixture->random_numbers_with_duplicates, random_numbers_with_duplicates, sizeof(random_numbers_with_duplicates));

    for (int i = 0; i < 128; i++) {
        fixture->random_numbers_index[i] = i;
        fixture->index_found[i] = 0;
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

static int verify_sorted_results(test_fixture_t* ctx) {

    for (int i = 0; i < 128 - 1; i++) {
        if (ctx->test_array[ctx->random_numbers_index[i]] > ctx->test_array[ctx->random_numbers_index[i + 1]]) {
            printf("Not sorted: %f >= %f at index %d and %d\n", ctx->test_array[ctx->random_numbers_index[i]], ctx->test_array[ctx->random_numbers_index[i + 1]], i, i + 1);
            return 0; // Not sorted
        }
        if (ctx->index_found[ctx->random_numbers_index[i]] == 1) {
            printf("Duplicate index found: %d at index %d\n", ctx->random_numbers_index[i], i);
            return 0; // Duplicate index found
        }
        if (ctx->with_duplicates == 0) {
            if (ctx->test_array[i] != ctx->random_numbers[i]) {
                printf("Non-indexed values altered: %f != %f at index %d\n", ctx->test_array[i], ctx->random_numbers[i], i);
                return 0; // Non-indexed values altered
            }
        }
        else {
            if (ctx->test_array[i] != ctx->random_numbers_with_duplicates[i]) {
                printf("Non-indexed values altered: %f != %f at index %d\n", ctx->test_array[i], ctx->random_numbers_with_duplicates[i], i);
                return 0; // Non-indexed values altered
            }
        }
        ctx->index_found[ctx->random_numbers_index[i]] = 1; // Mark index as found
    }
    if (ctx->index_found[ctx->random_numbers_index[127]] == 1) {
        printf("Duplicate index found: %d at index %d\n", ctx->random_numbers_index[127], 127);
        return 0; // Duplicate index found
    }
    return 1; // Sorted
}

static void test_bubblesort(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers, sizeof(ctx->random_numbers));
    ctx->with_duplicates = 0;
    indexed_bubble_sort(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);

}

static void test_bubblesort_with_duplicates(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers_with_duplicates, sizeof(ctx->random_numbers_with_duplicates));
    ctx->with_duplicates = 1;
    indexed_bubble_sort(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_merge_sort(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers, sizeof(ctx->random_numbers));
    ctx->with_duplicates = 0;

    indexed_merge_sort(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_merge_sort_with_duplicates(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers_with_duplicates, sizeof(ctx->random_numbers_with_duplicates));
    ctx->with_duplicates = 1;

    indexed_merge_sort(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_1v(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers, sizeof(ctx->random_numbers));
    ctx->with_duplicates = 0;

    indexed_bitonic_sort_1v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_1v_with_duplicates(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers_with_duplicates, sizeof(ctx->random_numbers_with_duplicates));
    ctx->with_duplicates = 1;

    indexed_bitonic_sort_1v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_2v(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers, sizeof(ctx->random_numbers));
    ctx->with_duplicates = 0;

    indexed_bitonic_sort_2v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_2v_with_duplicates(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers_with_duplicates, sizeof(ctx->random_numbers_with_duplicates));
    ctx->with_duplicates = 1;

    indexed_bitonic_sort_2v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_4v(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers, sizeof(ctx->random_numbers));
    ctx->with_duplicates = 0;

    indexed_bitonic_sort_4v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_4v_with_duplicates(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers_with_duplicates, sizeof(ctx->random_numbers_with_duplicates));
    ctx->with_duplicates = 1;

    indexed_bitonic_sort_4v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_8v(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers, sizeof(ctx->random_numbers));
    ctx->with_duplicates = 0;

    indexed_bitonic_sort_8v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_8v_with_duplicates(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers_with_duplicates, sizeof(ctx->random_numbers_with_duplicates));
    ctx->with_duplicates = 1;

    indexed_bitonic_sort_8v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_16v(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers, sizeof(ctx->random_numbers));
    ctx->with_duplicates = 0;

    indexed_bitonic_sort_16v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

static void test_indexed_bitonic_sort_16v_with_duplicates(void** state) {
    test_fixture_t* ctx = (test_fixture_t*)(*state);
    memcpy(ctx->test_array, ctx->random_numbers_with_duplicates, sizeof(ctx->random_numbers_with_duplicates));
    ctx->with_duplicates = 1;

    indexed_bitonic_sort_16v(ctx->test_array, ctx->random_numbers_index, ctx->temp_workspace, 128);
    
    int result = verify_sorted_results(ctx);
    assert_int_equal(result, 1);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_bubblesort, setup, teardown),
        cmocka_unit_test_setup_teardown(test_bubblesort_with_duplicates, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_merge_sort, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_merge_sort_with_duplicates, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_1v, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_1v_with_duplicates, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_2v, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_2v_with_duplicates, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_4v, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_4v_with_duplicates, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_8v, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_8v_with_duplicates, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_16v, setup, teardown),
        cmocka_unit_test_setup_teardown(test_indexed_bitonic_sort_16v_with_duplicates, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}