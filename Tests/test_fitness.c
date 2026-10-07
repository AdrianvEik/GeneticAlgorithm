#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../src/Utility/fitness.h"
#include "../src/Utility/flatten.h"
#include "../src/Utility/selection.h"
#include "../src/Utility/pop.h"
#include "../src/Utility/process.h"
#include "../src/Utility/mutation.h"

static void sort_reference(gene_pool_t* p) {
    for (uint32_t i = 0; i < p->individuals; ++i) {
        p->sorted_indexes[i] = i;
        for (uint32_t j = i; j > 0 && p->pop_result_set[p->sorted_indexes[j]] <
             p->pop_result_set[p->sorted_indexes[j-1]]; --j) {
            uint32_t t = p->sorted_indexes[j];
            p->sorted_indexes[j] = p->sorted_indexes[j-1];
            p->sorted_indexes[j-1] = t;
        }
    }
}

static void check_range(const double* f, const double* expected, uint32_t n) {
    double raw[16], normalized[16], weights[16], raw_before[16];
    uint32_t order[16], order_before[16];
    gene_pool_t p = {0};
    p.individuals = n; p.pop_result_set = raw; p.normalized_result_set = normalized;
    p.flatten_result_set = weights; p.sorted_indexes = order;
    memcpy(raw, f, n*sizeof(double)); memcpy(raw_before, f, n*sizeof(double));
    sort_reference(&p); memcpy(order_before, order, n*sizeof(uint32_t));
    process_normalize(&p);
    for (uint32_t i=0; i<n; ++i) {
        assert(isfinite(normalized[i]) && normalized[i]>=0 && normalized[i]<=1);
        if (expected) assert(fabs(normalized[i]-expected[i]) < 1e-14);
        if (i) assert(normalized[order[i]] >= normalized[order[i-1]]);
    }
    const double gains[] = {0, nextafter(0.0,1.0), 1, 1000, DBL_MAX};
    for (int method=0; method<6; ++method) for (unsigned g=0; g<5; ++g) {
        flatten_param_t param = {method, gains[g], method == 4 ? 0.5 : 0.0};
        process_flatten(&p, &param);
        for (uint32_t i=0; i<n; ++i) {
            assert(isfinite(weights[i]) && weights[i]>=0 && weights[i]<=1);
            if (i) assert(weights[order[i]] >= weights[order[i-1]]);
            if (method==3 || method==5) assert(weights[i]==normalized[i]);
        }
    }
    assert(memcmp(raw, raw_before, n*sizeof(double)) == 0);
    assert(memcmp(order, order_before, n*sizeof(uint32_t)) == 0);
}

static void check_normalization_and_flattening(void) {
    double f[] = {30, 10, 20, -DBL_MAX, DBL_MAX};
    double u[] = {1, 0, 0.5, 0, 1};
    check_range(f,u,5);
    double equal[] = {7,7,7,7}, ones[] = {1,1,1,1};
    check_range(equal,ones,4);
    double negatives[] = {-4,-3,-2,-1}, quarters[] = {0,1.0/3,2.0/3,1};
    check_range(negatives,quarters,4);
    double zeros[] = {0,0,0,0}; check_range(zeros,ones,4);
    double sentinels[] = {-DBL_MAX,DBL_MAX,-DBL_MAX,DBL_MAX};
    double ends[] = {0,1,0,1}; check_range(sentinels,ends,4);
    double one[] = {7}; check_range(one,ones,1);
    double all_min[] = {-DBL_MAX,-DBL_MAX}; check_range(all_min,zeros,2);
    double all_max[] = {DBL_MAX,DBL_MAX}; check_range(all_max,ones,2);
    const double near_max = nextafter(DBL_MAX,0.0);
    double wide[] = {-near_max,0,near_max}, mid[] = {0,0.5,1}; check_range(wide,mid,3);
    double nearby_scores[] = {1,nextafter(1,2),nextafter(nextafter(1,2),2)}; check_range(nearby_scores,mid,3);
    const double tiny = nextafter(0.0,1.0);
    double small[] = {0,tiny,2*tiny}; check_range(small,mid,3);
    double ordinary_tie[] = {-DBL_MAX,5,5,DBL_MAX};
    double tie_expected[] = {0,1,1,1}; check_range(ordinary_tie,tie_expected,4);
    gene_pool_t empty = {0}; process_normalize(&empty);

    gene_pool_t p = {0}; double norm[] = {0,0.25,0.5,1}, w[4];
    p.individuals=4; p.normalized_result_set=norm; p.flatten_result_set=w;
    flatten_param_t linear = {0, DBL_MAX, DBL_MAX}; process_flatten(&p,&linear);
    assert(w[0]==0.5 && w[1]==0.625 && w[2]==0.75 && w[3]==1);
    flatten_param_t logarithmic = {2,3,0}; process_flatten(&p,&logarithmic);
    assert(fabs(w[2]-log(2.5)/log(4.0)) < 1e-14);
    flatten_param_t exponential = {1,2,0}; process_flatten(&p,&exponential);
    assert(fabs(w[0]-exp(-2)) < 1e-14 && w[3]==1);
    assert(fitness_acceptance(0,tiny)==0.5);
    assert(fitness_acceptance(1,tiny)==1 && fitness_acceptance(-1,tiny)==0);
    assert(fabs(fitness_acceptance(1,10)-0.52497918747894)<1e-14);
}

static void check_storage_dedupe_and_reset(void) {
    runtime_param_t runtime = default_runtime_param();
    runtime.individuals=7; runtime.genes=17; runtime.elitism=2;
    gene_pool_t a={0}, b={0}; init_gene_pool(&a,&runtime); init_gene_pool(&b,&runtime);
    assert(a.normalized_result_set != b.normalized_result_set);
    assert(a.duplicate_flags != b.duplicate_flags && a.reseed_indexes != b.reseed_indexes);
    assert((uintptr_t)a.normalized_result_set % _Alignof(double) == 0);
    assert((uintptr_t)a.pop_param_double[0] % _Alignof(double) == 0);
    for(uint32_t i=0;i<7;++i) {
        a.pop_result_set[i]=5; a.sorted_indexes[i]=i;
        memset(a.pop_param_bin[i],0,a.individual_mem_size);
    }
    /* A,A,B,B,C,D,E: preserve adjacent scan, including the second SIMD block. */
    a.pop_param_bin[2][16]=a.pop_param_bin[3][16]=1;
    a.pop_param_bin[4][16]=2; a.pop_param_bin[5][16]=3; a.pop_param_bin[6][16]=4;
    double saved[7]; memcpy(saved,a.pop_result_set,sizeof saved);
    assert(dedupe_population(&a)==2);
    assert(a.duplicate_flags[0]==1 && a.duplicate_flags[2]==1);
    assert(a.duplicate_flags[1]==0 && a.duplicate_flags[3]==0);
    assert(memcmp(saved,a.pop_result_set,sizeof saved)==0);
    /* Identical genes with distinct scores are outside the dedupe filter. */
    a.pop_result_set[0]=4; assert(dedupe_population(&a)==1);
    assert(a.duplicate_flags[0]==0);
    a.normalized_result_set[0]=0.4; a.flatten_result_set[0]=0.7;
    a.reseed_count=2; a.reseed_indexes[0]=5;
    population_param_t population={0}; population.sampling_type=pop_uniform;
    fx_param_t fx={0}; seed_rand_task_threadlocal(112,0); fill_pop(&a,population,fx);
    assert(a.reseed_count==0);
    for(uint32_t i=0;i<7;++i) {
        assert(a.pop_result_set[i]==-DBL_MAX);
        assert(a.normalized_result_set[i]==0 && a.flatten_result_set[i]==0);
        assert(a.duplicate_flags[i]==0 && a.reseed_indexes[i]==0);
    }
    assert(b.normalized_result_set[0]==0);
    free_gene_pool(&a); free_gene_pool(&b);
}

static void counts_for(gene_pool_t* pool, selection_param_t* param, unsigned* counts) {
    memset(counts,0,(size_t)pool->individuals*sizeof(unsigned));
    seed_rand_task_threadlocal(9183,0);
    for(unsigned batch=0;batch<30000;++batch) {
        process_selection(pool,param);
        for(uint32_t i=0;i<pool->individuals-pool->elitism;++i) {
            assert(pool->selected_indexes[i]<pool->individuals);
            ++counts[pool->selected_indexes[i]];
        }
    }
}

static void check_distributions_and_caches(void) {
    runtime_param_t runtime=default_runtime_param(); runtime.individuals=4; runtime.genes=1; runtime.elitism=0;
    gene_pool_t p={0}; init_gene_pool(&p,&runtime); init_pre_compute_selection(&p);
    config_ga_t config=default_config(runtime); selection_param_t s=config.selection_param;
    double f[]={2,4,1,3}; memcpy(p.pop_result_set,f,sizeof f); sort_reference(&p); process_normalize(&p);
    flatten_param_t identity={5,1,0}; process_flatten(&p,&identity);
    unsigned counts[4];
    counts_for(&p,&s,counts); /* proportional weights [1/3,1,0,2/3] */
    assert(counts[2]==0 && abs((int)counts[0]-20000)<1000);
    assert(abs((int)counts[1]-60000)<1000 && abs((int)counts[3]-40000)<1000);
    s.selection_method=selection_method_rank; s.selection_prob_param=1;
    counts_for(&p,&s,counts); assert(counts[1]==120000);
    /* Change equality groups without changing cached parameters. */
    p.pop_result_set[3]=4; sort_reference(&p);
    counts_for(&p,&s,counts); assert(counts[0]==0 && counts[2]==0);
    assert(abs((int)counts[1]-60000)<1000 && abs((int)counts[3]-60000)<1000);
    for(uint32_t i=0;i<4;++i) p.pop_result_set[i]=0;
    sort_reference(&p); counts_for(&p,&s,counts);
    for(unsigned i=0;i<4;++i) assert(abs((int)counts[i]-30000)<1000);
    memcpy(p.pop_result_set,f,sizeof f); sort_reference(&p); process_normalize(&p);
    s.selection_rank_distr=1; s.selection_temp_param=nextafter(0.0,1.0);
    counts_for(&p,&s,counts); assert(counts[1]==120000);
    /* Identical coordinates give zero diversity and therefore uniform fallback.
       Mixing with a point-mass rank distribution gives probabilities .125,.625,.125,.125. */
    s.selection_method=selection_method_rank_space; s.selection_div_param=0.5;
    for(unsigned i=0;i<4;++i) p.pop_param_bin[i][0]=UINT32_MAX;
    counts_for(&p,&s,counts);
    assert(abs((int)counts[1]-75000)<1000);
    assert(abs((int)counts[0]-15000)<1000 && abs((int)counts[2]-15000)<1000);
    p.pop_param_bin[0][0]=0; s.selection_div_param=1;
    counts_for(&p,&s,counts); /* squared distances .5625,.0625,.0625,.0625 */
    assert(abs((int)counts[0]-90000)<1000);
    for(unsigned i=1;i<4;++i) assert(abs((int)counts[i]-10000)<1000);
    for(unsigned i=0;i<4;++i) assert(isfinite(distances[i]) && distances[i]<=1);
    /* Reuse TLS for a different population size and dimensions, same parameters. */
    gene_pool_t q={0}; runtime.individuals=3; runtime.genes=2; init_gene_pool(&q,&runtime);
    for(unsigned i=0;i<3;++i) {q.pop_result_set[i]=(double)i; q.sorted_indexes[i]=i;}
    process_normalize(&q); process_selection(&q,&s);
    for(unsigned i=0;i<3;++i) assert(q.selected_indexes[i]<3);
    process_selection(&p,&s);
    /* Uniform tied populations and tiny temperatures must terminate for all methods. */
    for(unsigned i=0;i<4;++i) {p.pop_result_set[i]=0; p.pop_param_bin[i][0]=0;}
    sort_reference(&p); process_normalize(&p); process_flatten(&p,&identity);
    for(int method=0;method<8;++method) {
        s.selection_method=method; counts_for(&p,&s,counts);
        for(unsigned i=0;i<4;++i) assert(abs((int)counts[i]-30000)<1600);
    }
    free_pre_compute_selection(); free_pre_compute_selection();
    assert(!prob_distr && !boltzmann_distr && !distances && !central_point && !rank_weights && !diversity_cdf);
    free_config_ga(&config); free_gene_pool(&p); free_gene_pool(&q);
}

static unsigned evaluations;
static double objective(void* parameters, uint32_t genes) {
    (void)genes; ++evaluations; return ((double*)parameters)[0];
}

static void check_pipeline(void) {
    /* 64 is an exact block for the existing sorter; its tail bug is separate. */
    runtime_param_t runtime=default_runtime_param(); runtime.individuals=64; runtime.genes=16; runtime.elitism=2;
    gene_pool_t p={0}; init_gene_pool(&p,&runtime); init_pre_compute_selection(&p);
    config_ga_t config=default_config(runtime);
    config.population_param.sampling_type=pop_uniform;
    config.population_param.reseed_bottom_N=100; /* clamp reseeding to non-elites */
    config.fx_param.fx_method=fx_method_pointer; config.fx_param.fx_function=objective;
    task_param_t task={0}; task.config_ga=config;
    task.lower=config.population_param.lower; task.upper=config.population_param.upper;
    fx_task_queue_t queue={0};
    seed_rand_task_threadlocal(789,0); fill_pop(&p,config.population_param,config.fx_param);
    init_mutation_rates(&p,0);
    for(unsigned generation=0;generation<3;++generation) {
        p.iteration_number=generation; evaluations=0;
        process_pop(&p,&task,&queue);
        assert(evaluations==62 && p.reseed_count==62);
        for(unsigned rank=0;rank<64;++rank) {
            const uint32_t i=p.sorted_indexes[rank]; assert(i<64);
            assert(isfinite(p.normalized_result_set[i]) && p.normalized_result_set[i]>=0 && p.normalized_result_set[i]<=1);
            if(rank) assert(p.pop_result_set[p.sorted_indexes[rank-1]]<=p.pop_result_set[i]);
            if(rank<62) assert(p.reseed_indexes[rank]==i);
        }
    }
    free_pre_compute_selection(); free_config_ga(&config); free_gene_pool(&p);
}

int main(void) {
#ifdef _MSC_VER
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    check_normalization_and_flattening();
    check_storage_dedupe_and_reset();
    check_distributions_and_caches();
    check_pipeline();
    puts("PASS: normalized fitness, flatteners, storage, dedupe, selection distributions/cache reuse, and population pipeline.");
    return 0;
}
