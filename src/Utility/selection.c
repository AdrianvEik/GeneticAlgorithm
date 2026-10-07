#include "selection.h"
#include "fitness.h"

/* Rejection removes modulo bias. All callers supply a positive bound. */
static uint32_t random_index(uint32_t bound) {
    const uint32_t threshold = (0u - bound) % bound;
    uint32_t value;
    do { value = gen_mt_rand(); } while (value < threshold);
    return value % bound;
}

static double random_unit(void) {
    return (double)gen_mt_rand() / 4294967296.0; /* [0,1), including zero */
}

/* Finite nonnegative weights are a precondition. Scaling before accumulation
   bounds the total by N. A zero total becomes a uniform distribution. */
static double build_cdf(const double* weights, double* cdf, uint32_t n) {
    double maximum = 0.0;
    for (uint32_t i = 0; i < n; ++i) if (weights[i] > maximum) maximum = weights[i];
    double total = 0.0;
    for (uint32_t i = 0; i < n; ++i) {
        total += maximum == 0.0 ? 1.0 : weights[i] / maximum;
        cdf[i] = total;
    }
    return total;
}

static uint32_t sample_cdf(const double* cdf, uint32_t n, double total) {
    double draw = random_unit() * total;
    if (draw >= total) draw = nextafter(total, 0.0);
    uint32_t lo = 0, hi = n - 1;
    /* First cumulative value strictly greater than draw: zero-weight entries
       cannot win at a boundary, including a draw of exactly zero. */
    while (lo < hi) {
        const uint32_t mid = lo + (hi - lo) / 2;
        if (cdf[mid] <= draw) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

static void roulette_selection(gene_pool_t* pool) {
    const double total = build_cdf(pool->flatten_result_set, pool->selection_temp, pool->individuals);
    for (uint32_t i = 0; i < pool->individuals - pool->elitism; ++i)
        pool->selected_indexes[i] = sample_cdf(pool->selection_temp, pool->individuals, total);
}

static void tournament_selection(gene_pool_t* pool, const selection_param_t* param) {
    for (uint32_t i = 0; i < pool->individuals - pool->elitism; ++i) {
        uint32_t best = random_index(pool->individuals), ties = 1;
        for (uint32_t draw = 1; draw < param->selection_tournament_size; ++draw) {
            const uint32_t candidate = random_index(pool->individuals);
            const double u = pool->normalized_result_set[candidate];
            const double best_u = pool->normalized_result_set[best];
            if (u > best_u || (u == best_u && pool->pop_result_set[candidate] > pool->pop_result_set[best])) {
                best = candidate;
                ties = 1;
            } else if (u == best_u && pool->pop_result_set[candidate] == pool->pop_result_set[best]) {
                /* Reservoir sampling gives each tied draw equal probability. */
                if (random_index(++ties) == 0) best = candidate;
            }
        }
        pool->selected_indexes[i] = best;
    }
}

static double prepare_rank_cdf(gene_pool_t* pool, const selection_param_t* param, int diversity) {
    prepare_selection_workspace(pool, diversity);
    const uint32_t n = pool->individuals;
    double* base;
    if (param->selection_rank_distr == 0) {
        if (current_prob_param != param->selection_prob_param) {
            prob_distr[n - 1] = 1.0;
            for (uint32_t r = n - 1; r > 0; --r)
                prob_distr[r - 1] = prob_distr[r] * (1.0 - param->selection_prob_param);
            current_prob_param = param->selection_prob_param;
        }
        base = prob_distr;
    } else {
        if (current_temp_param != param->selection_temp_param) {
            /* Avoid 1/T overflow when T is a positive subnormal. */
            const double ratio = param->selection_temp_param <= 1.0 / 745.0
                ? 0.0 : exp(-1.0 / param->selection_temp_param);
            boltzmann_distr[n - 1] = 1.0;
            for (uint32_t r = n - 1; r > 0; --r)
                boltzmann_distr[r - 1] = boltzmann_distr[r] * ratio;
            current_temp_param = param->selection_temp_param;
        }
        base = boltzmann_distr;
    }
    /* Canonically equal candidates share their group's total rank weight.
       This population-dependent adjustment is deliberately not cached. */
    for (uint32_t first = 0; first < n;) {
        uint32_t end = first + 1;
        double sum = base[first];
        while (end < n && pool->pop_result_set[pool->sorted_indexes[end]] ==
                          pool->pop_result_set[pool->sorted_indexes[first]]) sum += base[end++];
        const double average = sum / (double)(end - first);
        for (uint32_t r = first; r < end; ++r) rank_weights[r] = average;
        first = end;
    }
    return build_cdf(rank_weights, pool->selection_temp, n);
}

static void rank_selection(gene_pool_t* pool, const selection_param_t* param) {
    const double total = prepare_rank_cdf(pool, param, 0);
    for (uint32_t i = 0; i < pool->individuals - pool->elitism; ++i) {
        const uint32_t rank = sample_cdf(pool->selection_temp, pool->individuals, total);
        pool->selected_indexes[i] = pool->sorted_indexes[rank];
    }
}

static void compute_distances(gene_pool_t* pool) {
    /* Numeric uint32 chromosome coordinates mapped to [0,1]. This is an
       encoded-coordinate metric; categorical/phenotype metrics remain TODO. */
    memset(central_point, 0, (size_t)pool->genes * sizeof(double));
    memset(distances, 0, (size_t)pool->individuals * sizeof(double));
    for (uint32_t g = 0; g < pool->genes; ++g) {
        for (uint32_t i = 0; i < pool->individuals; ++i)
            central_point[g] += (double)pool->pop_param_bin[i][g] / (double)UINT32_MAX;
        central_point[g] /= (double)pool->individuals;
    }
    for (uint32_t i = 0; i < pool->individuals; ++i) {
        for (uint32_t g = 0; g < pool->genes; ++g) {
            const double diff = (double)pool->pop_param_bin[i][g] / (double)UINT32_MAX - central_point[g];
            distances[i] += diff * diff;
        }
        distances[i] /= (double)pool->genes;
    }
}

static void space_selection(gene_pool_t* pool, const selection_param_t* param) {
    const double rank_total = prepare_rank_cdf(pool, param, 1);
    compute_distances(pool);
    const double diversity_total = build_cdf(distances, diversity_cdf, pool->individuals);
    /* P(i) = (1-lambda) R_i/sum(R) + lambda D_i/sum(D).
       Choosing a component then sampling it realizes the same distribution.
       The rank CDF is in rank order; the diversity CDF is in physical order. */
    for (uint32_t i = 0; i < pool->individuals - pool->elitism; ++i) {
        if (random_unit() < param->selection_div_param)
            pool->selected_indexes[i] = sample_cdf(diversity_cdf, pool->individuals, diversity_total);
        else
            pool->selected_indexes[i] = pool->sorted_indexes[
                sample_cdf(pool->selection_temp, pool->individuals, rank_total)];
    }
}

static uint32_t choose_other(gene_pool_t* pool, uint32_t a, uint32_t b,
                             int strict, double threshold) {
    /* Goldberg's bounded search: try roughly a tenth of the population,
       at least once; use the last draw if no separated class was found.
       Strict differs from both classes; relaxed differs only from A. */
    const uint32_t attempts = pool->individuals / 10 > 0 ? pool->individuals / 10 : 1;
    uint32_t other = a;
    for (uint32_t j = 0; j < attempts; ++j) {
        other = random_index(pool->individuals);
        const double u = pool->normalized_result_set[other];
        if (fabs(u - pool->normalized_result_set[a]) > threshold &&
            (!strict || fabs(u - pool->normalized_result_set[b]) > threshold)) break;
    }
    return other;
}

static void boltzmann_selection(gene_pool_t* pool, const selection_param_t* param) {
    for (uint32_t i = 0; i < pool->individuals - pool->elitism; ++i) {
        const uint32_t a = random_index(pool->individuals);
        const uint32_t b = choose_other(pool, a, a, 0, param->selection_boltzmann_threshold);
        const int strict = param->selection_method == selection_method_boltzmann_strict ||
            (param->selection_method == selection_method_boltzmann && random_index(2) == 0);
        const uint32_t c = choose_other(pool, a, b, strict, param->selection_boltzmann_threshold);
        const double* u = pool->normalized_result_set;
        /* Anti-acceptance favors the weaker B/C; primary acceptance the stronger A/W. */
        const uint32_t secondary = random_unit() < fitness_acceptance(u[c] - u[b], param->selection_temp_param) ? b : c;
        pool->selected_indexes[i] = random_unit() < fitness_acceptance(u[a] - u[secondary], param->selection_temp_param)
            ? a : secondary;
    }
}

static void pairwise_selection(gene_pool_t* pool, const selection_param_t* param) {
    for (uint32_t i = 0; i < pool->individuals - pool->elitism; ++i) {
        const uint32_t a = random_index(pool->individuals);
        const uint32_t b = random_index(pool->individuals);
        const double difference = pool->normalized_result_set[a] - pool->normalized_result_set[b];
        pool->selected_indexes[i] = random_unit() < fitness_acceptance(difference, param->selection_temp_param) ? a : b;
    }
}

void process_selection(gene_pool_t* pool, selection_param_t* param) {
    if (pool->individuals == 0 || pool->individuals == pool->elitism) return;
    if (param->selection_method == selection_method_roulette) roulette_selection(pool);
    else if (param->selection_method == selection_method_rank_tournament) tournament_selection(pool, param);
    else if (param->selection_method == selection_method_rank) rank_selection(pool, param);
    else if (param->selection_method == selection_method_rank_space) space_selection(pool, param);
    else if (param->selection_method == selection_method_boltzmann ||
             param->selection_method == selection_method_boltzmann_strict ||
             param->selection_method == selection_method_boltzmann_relaxed) boltzmann_selection(pool, param);
    else if (param->selection_method == selection_method_logistic_pairwise) pairwise_selection(pool, param);
    else EXIT_WITH_ERROR("Unknown selection method", 1);
}
