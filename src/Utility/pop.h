
#ifndef POP_H
#define POP_H
#include "../Helper/Struct.h"
#include "../Helper/rng.h"


void init_gene_pool(gene_pool_t* gene_pool);
void free_gene_pool(gene_pool_t* gene_pool);

void fill_pop(gene_pool_t* gene_pool);
void fill_individual(gene_pool_t* gene_pool, int individual, mt_rand_t* mt_rand);

// void bitpop(int bitsize, int genes, int individuals, int** result);
// void bitpop32(int genes, int individuals, int** result);

// void uniform_bit_pop(int bitsize, int genes, int individuals, double factor, double bias, int** result);
// void normal_bit_pop(int bitsize, int genes, int individuals, double factor, double bias, double loc, double scale, int** result);
// void normal_bit_pop_boxmuller(int bitsize, int genes, int individuals, double factor, double bias, double loc, double scale, int** result);
// void cauchy_bit_pop(int bitsize, int genes, int individuals, double factor, double bias, double loc, double scale, int** result);

#endif