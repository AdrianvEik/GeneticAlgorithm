#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "../Helper/Helper.h"
#include "../Helper/Struct.h"

#include "mutation.h"

void mutate32(gene_pool_t *gene_pool, mutation_param_t *mutation_param){

    /*
    
    This function mutates a bitarray by flipping a random bit.

    :param bit: bitarray to mutate
    :type bit: int*

    :param size: size of the bitarray
    :type size: int

    :param mutate_coeff_rate: amount of mutations over the bitarray
    :type mutate_coeff_rate: int

    :param chaos_coeff: the signifigance of the bits impacted by the mutation (1 to 32) (1 for least significant bit, 32 for most significant bit)
    :type chaos_coeff: int

    :param allow_sign_flip: whether or not to allow the sign to flip, 1 for yes, 0 for no
    :type allow_sign_flip: int

    */

    // int* mutations = malloc(gene_pool->genes * sizeof(int));
    // // generate random mutations, that are not at the same position
    // for (int i = 0; i < gene_pool->genes; i++){
    //     mutations[i] = 0;
    // }

    int mutation_gene;
    int mutation_bit = 0;
    for (int i = 0; i < gene_pool->individuals-gene_pool->elitism; i++){
        for (int j = 0; j < mutation_param->mutation_rate; j++){ // check if works
            // ensure that the selected gene is positive
            mutation_gene = (random_intXOR32() & 0x7fffffff) % gene_pool->genes;
            mutation_bit = (int) 1 << (random_intXOR32() % sizeof(int)*8);
            gene_pool->pop_param_bin[gene_pool->sorted_indexes[i]][mutation_gene] ^= mutation_bit;
        }

    }

}

void mutate(gene_pool_t *gene_pool, mutation_param_t *mutation_param){

    /*
    
    This function mutates a bitarray by flipping a random bit.

    :param bit: bitarray to mutate
    :type bit: int*

    :param size: size of the bitarray
    :type size: int

    :param mutate_coeff_rate: amount of mutations over the bitarray
    :type mutate_coeff_rate: int
    */

    // mutate_coeff_rate is the amount of mutations over the bit;
    // check if mutate_coeff_rate is < size

    // if (mutate_coeff_rate > genes){
    //     printf("Error: mutate_coeff_rate is bigger than size\n");
    //     exit(1);
    // }

    // int *mutations = malloc(genes * sizeof(int));
    // // generate random mutations, that are not at the same position
    // for (int i = 0; i < genes; i++){
    //     mutations[i] = rand() % genes;
    //     for (int j = 0; j < i; j++){
    //         if (mutations[i] == mutations[j]){
    //             i--;
    //             break;
    //         }
    //     }
    // }
    
    // // mutate the bit
    // for (int i = 0; i < mutate_coeff_rate; i++){
    //     individual[mutations[i]] = !individual[mutations[i]];
    // }

    // free(mutations);

}