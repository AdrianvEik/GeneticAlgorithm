#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/Function/Benchmarks.h"

static void check_close(double actual, double expected, double tolerance) {
    if (!isfinite(actual) || fabs(actual - expected) > tolerance) {
        fprintf(stderr, "Expected %.17g, got %.17g\n", expected, actual);
        exit(EXIT_FAILURE);
    }
}

static void check_nan(double actual) {
    if (!isnan(actual)) {
        fprintf(stderr, "Expected NAN, got %.17g\n", actual);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    double zero[32] = {0};
    double ones[32];
    double optimum[32];
    for (unsigned i = 0; i < 32; i++) {
        ones[i] = 1.0;
        optimum[i] = 420.9687462275036;
    }
    /* Verify minima and dimension scaling, including the one-dimensional case. */
    for (uint32_t d = 1; d <= 32; d++) {
        check_close(Ackley_fx(zero, d), 0.0, 1e-12);
        check_close(Griewank_fx(zero, d), 0.0, 1e-12);
        check_close(Levy_fx(ones, d), 0.0, 1e-12);
        check_close(Rastrigin_fx(zero, d), 0.0, 1e-12);
        check_close(Rastrigin_fx(ones, d), (double)d, 1e-12);
        check_close(Schwefel_fx(optimum, d), 1.2727566e-5 * d, 1e-9);
        check_close(Schwefel_fx(zero, d), 418.9829 * d, 1e-10);
    }
    /* Non-optimal points catch sign, indexing and trigonometric phase errors. */
    double point[] = {1.0, 2.0};
    check_close(Ackley_fx(point, 2), 5.422131717799505, 1e-12);
    check_close(Griewank_fx(point, 2), 1.00125 - cos(1.0) * cos(sqrt(2.0)), 1e-12);
    check_close(Levy_fx(zero, 1), 0.625, 1e-12);
    check_close(Levy_fx(zero, 2), 0.7158445541169746, 1e-12);
    check_close(Rastrigin_fx(point, 2), 5.0, 1e-12);
    check_close(Ackley_param_fx(point, 2, 3.0, 0.0, 0.0), 0.0, 1e-12);

    double center[] = {2, 1};
    const double pi = 3.14159265358979323846;
    check_close(Langermann_fx(center, 2),
                5.0 - exp(-17.0 / pi) + 4.0 * exp(-10.0 / pi)
                - 3.0 * exp(-89.0 / pi), 1e-12);
    double cvec[] = {2.0, 3.0};
    double A[] = {0, 0, 0, 1, 0, 0};
    check_close(Langermann_param_fx(zero, 3, 2, cvec, A),
                2.0 - 3.0 * exp(-1.0 / 3.14159265358979323846), 1e-12);
    double center_A[] = {0, 0, 0, 0, 0};
    check_close(Langermann_param_fx(zero, 1, 5, NULL, center_A), 13.0, 1e-12);
    double default_c[] = {1, 2, 5, 2, 3};
    check_close(Langermann_param_fx(center, 2, 5, default_c, NULL),
                Langermann_fx(center, 2), 1e-12);

    double (*functions[])(double*, uint32_t) = {
        Ackley_fx, Griewank_fx, Langermann_fx, Levy_fx, Rastrigin_fx, Schwefel_fx
    };
    for (unsigned i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
        check_nan(functions[i](NULL, 2));
        check_nan(functions[i](zero, 0));
    }
    check_nan(Langermann_fx(zero, 3));
    check_nan(Langermann_param_fx(zero, 2, 2, NULL, A));
    check_nan(Langermann_param_fx(zero, 3, 2, cvec, NULL));
    check_nan(Langermann_param_fx(zero, 2, 0, cvec, A));
    puts("Benchmark tests passed");
    return EXIT_SUCCESS;
}
