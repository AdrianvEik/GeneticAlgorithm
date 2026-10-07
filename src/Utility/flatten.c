#include "flatten.h"
#include "fitness.h"

void process_flatten(gene_pool_t* pool, flatten_param_t* param) {
    /* Preconditions (validation deferred to the project-wide parameter sweep):
       finite alpha >= 0; linear beta >= 0; sigmoid beta in [0,1].
       Exp/log use alpha only; none/normalized ignore both coefficients. */
    const double alpha = param->flatten_alpha;
    const double beta = param->flatten_beta;
    double linear_a = 0.0, linear_b = 0.0;
    if (param->flatten_method == flatten_method_linear) {
        /* Weights proportional to alpha*u + beta, scaled before addition.
           Divide by the maximum coefficient, then by the bounded sum. */
        const double scale = fmax(alpha, beta);
        if (scale > 0.0) {
            linear_a = alpha / scale;
            linear_b = beta / scale;
            const double sum = linear_a + linear_b;
            linear_a /= sum;
            linear_b /= sum;
        }
    }
    for (uint32_t i = 0; i < pool->individuals; ++i) {
        const double u = pool->normalized_result_set[i];
        double w;
        if (param->flatten_method == flatten_method_linear) {
            w = linear_a * u + linear_b;
        } else if (param->flatten_method == flatten_method_exponential) {
            w = exp(alpha * (u - 1.0));
        } else if (param->flatten_method == flatten_method_logarithmic) {
            /* The linear limit also avoids alpha*u underflow for tiny alpha. */
            w = alpha <= 1e-8 ? u : log1p(alpha * u) / log1p(alpha);
        } else if (param->flatten_method == flatten_method_sigmoid) {
            w = fitness_sigmoid(alpha * (u - beta));
        } else if (param->flatten_method == flatten_method_none ||
                   param->flatten_method == flatten_method_normalized) {
            w = u;
        } else {
            EXIT_WITH_ERROR("Unknown flatten method", 1);
        }
        pool->flatten_result_set[i] = w;
    }
}
