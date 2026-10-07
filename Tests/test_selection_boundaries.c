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
#include "../src/Utility/selection.h"
#include "../src/Utility/fitness.h"

/* This target compiles the production selection.c with gen_mt_rand renamed.
   All candidate/coin/interval boundaries can therefore be driven exactly. */
static const uint32_t* draws;
static size_t draws_count, position;
uint32_t test_selection_random(void) {
    assert(position<draws_count);
    return draws[position++];
}

static void run(gene_pool_t* p, selection_param_t* s, const uint32_t* values, size_t n) {
    draws=values; draws_count=n; position=0;
    process_selection(p,s);
    assert(position==n);
}

int main(void) {
#ifdef _MSC_VER
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    double f[]={2,4,1,3}, u[4], w[4], temp[4];
    uint32_t order[]={2,0,3,1}, selected[4];
    gene_pool_t p={0}; p.individuals=4; p.genes=1;
    p.pop_result_set=f; p.normalized_result_set=u; p.flatten_result_set=w;
    p.selection_temp=temp; p.sorted_indexes=order; p.selected_indexes=selected;
    process_normalize(&p);
    selection_param_t s={0}; s.selection_temp_param=1; s.selection_tournament_size=2;
    const uint32_t boundaries[]={0,UINT32_MAX,0x80000000u,0x7fffffffu};
    w[0]=0; w[1]=1; w[2]=0; w[3]=1;
    run(&p,&s,boundaries,4);
    assert(selected[0]==1 && selected[1]==3 && selected[2]==3 && selected[3]==1);
    for(unsigned i=0;i<4;++i) w[i]=DBL_MAX;
    run(&p,&s,boundaries,4);
    assert(selected[0]==0 && selected[1]==3 && selected[2]==2 && selected[3]==1);
    memset(w,0,sizeof w); run(&p,&s,boundaries,4);
    assert(selected[0]==0 && selected[1]==3 && selected[2]==2 && selected[3]==1);
    s.selection_method=selection_method_rank; s.selection_prob_param=1;
    run(&p,&s,boundaries,4); for(unsigned i=0;i<4;++i) assert(selected[i]==1);
    /* Genuine normalization collision: nearby small scores amid a huge range. */
    f[0]=0; f[1]=0.5; f[2]=nextafter(0.5,1); f[3]=nextafter(DBL_MAX,0);
    for(unsigned i=0;i<4;++i) order[i]=i;
    process_normalize(&p); assert(u[1]==u[2]);
    p.elitism=3; s.selection_method=selection_method_rank_tournament;
    const uint32_t competitors[]={1,2}; run(&p,&s,competitors,2); assert(selected[0]==2);
    f[1]=f[2]=1; process_normalize(&p);
    const uint32_t tie[]={1,2,0}; run(&p,&s,tie,3); assert(selected[0]==2);

    u[0]=0; u[1]=0.25; u[2]=0.5; u[3]=1;
    s.selection_method=selection_method_boltzmann;
    /* A=.5 B=0 C=1: anti-acceptance at draw .5 keeps B, then .6 keeps A.
       Using ordinary acceptance in the first round would incorrectly choose C. */
    const uint32_t mixed[]={2,0,0,3,0x80000000u,0x99999999u};
    run(&p,&s,mixed,6); assert(selected[0]==2);
    const uint32_t fixed[]={2,0,3,0x80000000u,0x99999999u};
    s.selection_method=selection_method_boltzmann_strict; run(&p,&s,fixed,5); assert(selected[0]==2);
    s.selection_method=selection_method_boltzmann_relaxed; run(&p,&s,fixed,5); assert(selected[0]==2);
    const uint32_t relaxed_mix[]={2,0,1,3,0x80000000u,0x99999999u};
    s.selection_method=selection_method_boltzmann; run(&p,&s,relaxed_mix,6); assert(selected[0]==2);
    /* No separated classes: bounded fallback must terminate with equal fitness. */
    for(unsigned i=0;i<4;++i) u[i]=1;
    run(&p,&s,mixed,6);
    s.selection_method=selection_method_logistic_pairwise;
    u[0]=0;u[3]=1;
    const uint32_t pair[]={0,3,0x80000000u}; run(&p,&s,pair,3); assert(selected[0]==3);
    s.selection_temp_param=nextafter(0.0,1.0); run(&p,&s,pair,3); assert(selected[0]==3);
    /* With two search attempts, strict must reject B's class for C whereas
       relaxed may accept it. Bound=20 rejects RNG words below 16, so use
       20+index for scripted candidate draws. */
    double classes[20]={0}; uint32_t choices[20];
    gene_pool_t q={0}; q.individuals=20; q.elitism=19;
    q.normalized_result_set=classes; q.selected_indexes=choices;
    classes[2]=0.5; classes[3]=1;
    s.selection_temp_param=1; s.selection_method=selection_method_boltzmann_strict;
    const uint32_t strict_retry[]={22,20,20,23,0x80000000u,0x99999999u};
    run(&q,&s,strict_retry,6); assert(choices[0]==2);
    s.selection_method=selection_method_boltzmann_relaxed;
    const uint32_t relaxed_accept[]={22,20,20,0x80000000u,0x99999999u};
    run(&q,&s,relaxed_accept,5); assert(choices[0]==2);
    s.selection_method=selection_method_logistic_pairwise;
    /* Degenerate internal calls return valid indexes, or perform no draws. */
    p.individuals=1;p.elitism=0; const uint32_t singleton[]={0,0,0};
    run(&p,&s,singleton,3); assert(selected[0]==0);
    p.elitism=1; run(&p,&s,NULL,0);
    p.individuals=0;p.elitism=0; run(&p,&s,NULL,0);
    free_pre_compute_selection();
    puts("PASS: exact roulette boundaries, ascending rank mapping, tournament ties, and all Boltzmann contest directions.");
    return 0;
}
