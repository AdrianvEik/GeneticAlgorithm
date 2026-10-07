#include "Benchmarks.h"
#include "../Helper/error_handling.h"

#include <math.h>
#include <stddef.h>

static const double benchmark_pi = 3.14159265358979323846;

double Styblinski_Tang_fx(double* parameter_set, uint32_t genes) {
    double result = 0;
    for (uint32_t i = 0; i < genes; i++) {
        result += pow(parameter_set[i], 4) - 16 * pow(parameter_set[i], 2)
                  + 5 * parameter_set[i];
    }
    return result / 2;
}

double wheelers_ridge_fx(double* parameter_set, uint32_t genes) {
    double a = 1.5;
    if (genes != 2) EXIT_WITH_ERROR("Genes must be 2 for Wheelers Ridge", 255);

    double x1 = parameter_set[0];
    double x2 = parameter_set[1];
    return -1 * exp(-1 * pow(x1 * x2 - a, 2) - pow(x2 - a, 2));
}

double Ackley_param_fx(const double* parameter_set, uint32_t genes,
                      double a, double b, double c) {
    if (parameter_set == NULL || genes == 0) return NAN;
    double sum1 = 0.0;
    double sum2 = 0.0;
    for (uint32_t i = 0; i < genes; i++) {
        sum1 += parameter_set[i] * parameter_set[i];
        sum2 += cos(c * parameter_set[i]);
    }
    return -a * exp(-b * sqrt(sum1 / genes)) - exp(sum2 / genes) + a + exp(1.0);
}

double Ackley_fx(double* parameter_set, uint32_t genes) {
    return Ackley_param_fx(parameter_set, genes, 20.0, 0.2, 2.0 * benchmark_pi);
}

double Griewank_fx(double* parameter_set, uint32_t genes) {
    if (parameter_set == NULL || genes == 0) return NAN;
    double sum = 0.0;
    double product = 1.0;
    for (uint32_t i = 0; i < genes; i++) {
        sum += parameter_set[i] * parameter_set[i] / 4000.0;
        product *= cos(parameter_set[i] / sqrt((double)i + 1.0));
    }
    return sum - product + 1.0;
}

double Langermann_param_fx(const double* parameter_set, uint32_t genes,
                          uint32_t m, const double* cvec, const double* A) {
    static const double default_cvec[] = {1, 2, 5, 2, 3};
    static const double default_A[] = {3, 5, 5, 2, 2, 1, 1, 4, 7, 9};
    if (parameter_set == NULL || genes == 0 || m == 0) return NAN;
    if (cvec == NULL) {
        if (m != 5) return NAN;
        cvec = default_cvec;
    }
    if (A == NULL) {
        if (m != 5 || genes != 2) return NAN;
        A = default_A;
    }
    double result = 0.0;
    for (uint32_t i = 0; i < m; i++) {
        double inner = 0.0;
        for (uint32_t j = 0; j < genes; j++) {
            double delta = parameter_set[j] - A[(size_t)i * genes + j];
            inner += delta * delta;
        }
        result += cvec[i] * exp(-inner / benchmark_pi) * cos(benchmark_pi * inner);
    }
    return result;
}

double Langermann_fx(double* parameter_set, uint32_t genes) {
    return Langermann_param_fx(parameter_set, genes, 5, NULL, NULL);
}

double Levy_fx(double* parameter_set, uint32_t genes) {
    if (parameter_set == NULL || genes == 0) return NAN;
    double first = sin(benchmark_pi * (1.0 + (parameter_set[0] - 1.0) / 4.0));
    double result = first * first;
    for (uint32_t i = 0; i < genes - 1; i++) {
        double w = 1.0 + (parameter_set[i] - 1.0) / 4.0;
        double s = sin(benchmark_pi * w + 1.0);
        result += (w - 1.0) * (w - 1.0) * (1.0 + 10.0 * s * s);
    }
    double last = 1.0 + (parameter_set[genes - 1] - 1.0) / 4.0;
    double s = sin(2.0 * benchmark_pi * last);
    return result + (last - 1.0) * (last - 1.0) * (1.0 + s * s);
}

double Rastrigin_fx(double* parameter_set, uint32_t genes) {
    if (parameter_set == NULL || genes == 0) return NAN;
    double result = 10.0 * genes;
    for (uint32_t i = 0; i < genes; i++) {
        result += parameter_set[i] * parameter_set[i]
                  - 10.0 * cos(2.0 * benchmark_pi * parameter_set[i]);
    }
    return result;
}

double Schwefel_fx(double* parameter_set, uint32_t genes) {
    if (parameter_set == NULL || genes == 0) return NAN;
    double sum = 0.0;
    for (uint32_t i = 0; i < genes; i++) {
        sum += parameter_set[i] * sin(sqrt(fabs(parameter_set[i])));
    }
    return 418.9829 * genes - sum;
}
