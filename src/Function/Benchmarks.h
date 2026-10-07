#ifndef GA_BENCHMARKS_H
#define GA_BENCHMARKS_H

#include <stdint.h>

/**
 * Evaluate Styblinski-Tang: sum(x_i^4 - 16*x_i^2 + 5*x_i)/2.
 *
 * :param parameter_set: Array of genes coordinates.
 * :param genes: Dimension d. A zero-dimensional sum returns zero.
 * :returns: Objective value before the optimization-mode sign is applied.
 */
double Styblinski_Tang_fx(double* parameter_set, uint32_t genes);

/**
 * Evaluate Wheeler's Ridge with a=1.5:
 * -exp(-(x1*x2-a)^2 - (x2-a)^2).
 *
 * :param parameter_set: Array containing x1 and x2.
 * :param genes: Must equal two; other dimensions terminate with error 255,
 *     preserving the original function's error handling.
 * :returns: Objective value before the optimization-mode sign is applied.
 */
double wheelers_ridge_fx(double* parameter_set, uint32_t genes);

/**
 * Evaluate Ackley with a=20, b=0.2, c=2*pi and d=genes >= 1.
 *
 * Recommended domain: [-32.768, 32.768]^d. Global minimum f(0,...,0)=0
 * for every d. SFU does not specify local extrema or a maximum.
 * Returns NAN for a NULL parameter_set or zero genes.
 * Source: https://www.sfu.ca/~ssurjano/ackley.html
 */
double Ackley_fx(double* parameter_set, uint32_t genes);

/**
 * Evaluate Ackley with explicit a, b and c (defaults: 20, 0.2, 2*pi).
 *
 * The domain and minimum documented for Ackley_fx use the default constants;
 * arbitrary constants may change the extrema. Returns NAN for NULL input or
 * zero genes. parameter_set contains genes coordinates.
 */
double Ackley_param_fx(const double* parameter_set, uint32_t genes,
                      double a, double b, double c);

/**
 * Evaluate Griewank for d=genes >= 1.
 *
 * Recommended domain: [-600, 600]^d. Global minimum f(0,...,0)=0
 * for every d. SFU does not specify local extrema or a maximum.
 * Returns NAN for a NULL parameter_set or zero genes.
 * Source: https://www.sfu.ca/~ssurjano/griewank.html
 */
double Griewank_fx(double* parameter_set, uint32_t genes);

/**
 * Evaluate Langermann with d=genes=2, m=5, cvec=(1,2,5,2,3) and
 * A rows (3,5), (5,2), (2,1), (1,4), (7,9).
 *
 * Recommended domain: [0,10]^d. SFU specifies neither a global extremum nor
 * local extrema or a maximum; extrema depend on m, cvec and A, not d alone.
 * Uses the positive sum in the supplied SFU R implementation.
 * Returns NAN for NULL input or genes != 2. Use Langermann_param_fx for
 * other dimensions or constants.
 * Source: https://www.sfu.ca/~ssurjano/langer.html
 */
double Langermann_fx(double* parameter_set, uint32_t genes);

/**
 * Evaluate Langermann for d=genes >= 1 and m >= 1.
 *
 * parameter_set contains d coordinates; cvec contains m coefficients;
 * A contains m*d entries in row-major order. NULL cvec selects (1,2,5,2,3)
 * only when m=5; NULL A selects the matrix in Langermann_fx only when
 * m=5 and d=2. Other missing defaults, NULL input, zero genes or zero m
 * return NAN. Extrema depend on these parameters; SFU gives no formula
 * for them as a function of d. Recommended domain: [0,10]^d.
 */
double Langermann_param_fx(const double* parameter_set, uint32_t genes,
                          uint32_t m, const double* cvec, const double* A);

/**
 * Evaluate Levy for d=genes >= 1, with w_i=1+(x_i-1)/4.
 *
 * Recommended domain: [-10,10]^d. Global minimum f(1,...,1)=0 for every d.
 * For d=1 the intermediate sum is empty. SFU does not specify local
 * extrema or a maximum. Returns NAN for NULL input or zero genes.
 * Source: https://www.sfu.ca/~ssurjano/levy.html
 */
double Levy_fx(double* parameter_set, uint32_t genes);

/**
 * Evaluate Rastrigin for d=genes >= 1.
 *
 * Recommended domain: [-5.12,5.12]^d. Global minimum f(0,...,0)=0
 * for every d. SFU does not specify local extrema or a maximum.
 * Returns NAN for a NULL parameter_set or zero genes.
 * Source: https://www.sfu.ca/~ssurjano/rastr.html
 */
double Rastrigin_fx(double* parameter_set, uint32_t genes);

/**
 * Evaluate Schwefel for d=genes >= 1 using the supplied constant 418.9829.
 *
 * Recommended domain: [-500,500]^d. SFU reports a global minimum of zero
 * at x_i=420.9687 for all i. With the rounded constant used here, the
 * minimum is approximately 1.2727566e-5*d at x_i approximately 420.968746.
 * This minimum is for the stated domain. SFU does not specify local
 * extrema or a maximum. Returns NAN for NULL input or zero genes.
 * Source: https://www.sfu.ca/~ssurjano/schwef.html
 */
double Schwefel_fx(double* parameter_set, uint32_t genes);

#endif
