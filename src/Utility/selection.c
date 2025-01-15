#include "stdio.h"
#include "stdlib.h"
#include <math.h>
#include <windows.h>
#include <pthread.h>


#include "selection.h"
#include "../Multiprocessing/mp_thread_locals.h"
// Maybe we can use this in rng too?


void compute_distr(gene_pool_t* gene_pool, selection_param_t* selection_param) {
    /*
    */
    double sum = 0;
    current_prob_param = selection_param->selection_prob_param;
    for (int i = 0; i < gene_pool->individuals; i++) {
        prob_distr[i] = selection_param->selection_prob_param * pow((1 - selection_param->selection_prob_param), i - 1);
        sum += prob_distr[i];
    }
    for (int i = 0; i < gene_pool->individuals; i++) {
        prob_distr[i] /= sum;
    }
}

void compute_boltzmann_distr(gene_pool_t* gene_pool, selection_param_t* selection_param) {
    /*
    */
    double sum = 0;
    current_temp_param = selection_param->selection_temp_param;
    for (int i = 0; i < gene_pool->individuals; i++) {
        boltzmann_distr[i] = exp(-i / selection_param->selection_temp_param);
        sum += boltzmann_distr[i];
    }
    for (int i = 0; i < gene_pool->individuals; i++) {
        boltzmann_distr[i] /= sum;
    }
}

inline void compute_distances(gene_pool_t* gene_pool) {
    /*
    */
    // Compute the central point of the distribution as a vector
    if (central_point == NULL) {
        central_point = (double*)malloc(gene_pool->genes * sizeof(double));
        if (central_point == NULL) {
            printf("Memory allocation failed: compute_distances");
            exit(255);
        }
    }

    if (distances == NULL) {
        distances = (double*)malloc(gene_pool->individuals * sizeof(double));
        if (distances == NULL) {
            printf("Memory allocation failed: compute_distances");
            exit(255);
        }
    }


    for (int i = 0; i < gene_pool->genes; i++) {
        central_point[i] = 0;
        for (int j = 0; j < gene_pool->individuals; j++) {
            central_point[i] += gene_pool->pop_param_double[gene_pool->selected_indexes[j]][i];
        }
        central_point[i] /= gene_pool->individuals;
    }

    // Compute the distance of each individual to the central point
    for (int i = 0; i < gene_pool->individuals; i++) {
        distances[i] = 0;
        for (int j = 0; j < gene_pool->genes; j++) {
            distances[i] += pow(gene_pool->pop_param_double[gene_pool->selected_indexes[i]][j] - central_point[j], 2);
        }
        distances[i] = sqrt(distances[i]);
    }

}

// Selection functions
static void roulette_selection(gene_pool_t* gene_pool, selection_param_t* selection_param) {
	/*

	:param pop: matrix of normalised fitness values for the population (individuals x 1)
	:param individuals: number of individuals
	:param genes: number of genes
	:param selected: matrix of the indices of selected individuals (individuals x 2)

	*/

	// select individuals
	roulette_wheel(gene_pool->flatten_result_set, gene_pool->individuals, gene_pool->individuals - gene_pool->elitism, gene_pool->selected_indexes);
}

static void tournament_selection(gene_pool_t* gene_pool, selection_param_t* selection_param) {
	/*
	*/
    // We hold n tournaments and select the best individual from each tournament
    for (int i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
        int best = -1;
        double best_fitness = -1;
        for (int j = 0; j < selection_param->selection_tournament_size; j++) {
            int index = gen_mt_rand() % (gene_pool->individuals);
            if (best == -1 || gene_pool->flatten_result_set[index] > best_fitness) {
                best = index;
                best_fitness = gene_pool->flatten_result_set[index];
            }
        }
        gene_pool->selected_indexes[i] = best;
    }
}


static void rank_selection(gene_pool_t* gene_pool, selection_param_t* selection_param) {
	/*

	*/
    // The individuals are already sorted in ascending order so we can just use the indexes
	roulette_wheel(selection_param->selection_rank_distr == 0 ? prob_distr : boltzmann_distr,
        gene_pool->individuals,
        gene_pool->individuals - gene_pool->elitism,
        gene_pool->selected_indexes
    );
}

static void space_selection(gene_pool_t* gene_pool, selection_param_t* selection_param) {
	/*

	*/
    // Compute the central point of the distribution as a vector
    compute_distances(gene_pool);

    // Using the fitness values plus distance times the distance parameter as the selection probability
    double* selection_prob = (double*)malloc(gene_pool->individuals * sizeof(double));
    if (selection_prob == NULL) {
        printf("Memory allocation failed");
        exit(255);
    }

    for (int i = 0; i < gene_pool->individuals; i++) {
        // For now it is not "rank" maybe this should be a seperate function
        selection_prob[i] = gene_pool->flatten_result_set[gene_pool->selected_indexes[i]] + selection_param->selection_div_param * distances[i];
    }

    // Now we can use the roulette wheel selection
    roulette_wheel(selection_prob, gene_pool->individuals, gene_pool->individuals - gene_pool->elitism, gene_pool->selected_indexes);

    // clear arrays to 0
    memset(central_point, 0, gene_pool->genes * sizeof(double));
    memset(distances, 0, gene_pool->individuals * sizeof(double));

    free(selection_prob);
}

static void boltzmann_selection(gene_pool_t* gene_pool, selection_param_t* selection_param) {
	/*
        TODO: Boltzmann selection currently uses flattened fitness values, should be able to use boltzmann distribution
	*/

    // We hold n selection rounds
    for (int i = 0; i < gene_pool->individuals - gene_pool->elitism; i++) {
        // Select a main competitor at random
        int main_competitor = gen_mt_rand() % (gene_pool->individuals);

        // Pick a distance from the main competitor at least two spaces away
        int distance = 2 + gen_mt_rand() % (gene_pool->individuals - 2);

        // Pick the second competitor < distance away
        int second_competitor = (main_competitor + (gen_mt_rand() % distance)) % (gene_pool->individuals);

        // Strict selection uses the same policy as the second competitor but is not the second competitor
        // Relaxed selection can pick any competitor that is not the second or the first

        int third_competitor;
        // Use a coinflip to decide if we have strict or relaxed selection
        if (gen_mt_rand() < 1 / 2 * 0xffffffff) { // strict
            third_competitor = (main_competitor + (gen_mt_rand() % distance)) % (gene_pool->individuals);
        }
        else { // relaxed
            third_competitor = gen_mt_rand() % (gene_pool->individuals);
            while (third_competitor == main_competitor || third_competitor == second_competitor) {
                third_competitor = gen_mt_rand() % (gene_pool->individuals);
            }
        }

        // Now we can have the anti acceptance competition between 2 and 3
        double acceptance[2];
        acceptance[0] = exp((gene_pool->flatten_result_set[second_competitor] - gene_pool->flatten_result_set[third_competitor]) / selection_param->selection_temp_param);
        acceptance[1] = 1 - acceptance[0];
        int winner_anti_acceptance;
        roulette_wheel(acceptance, 2, 1, &winner_anti_acceptance);

        // The winner of the anti acceptance competition competes against the main competitor
        acceptance[0] = exp((gene_pool->flatten_result_set[main_competitor] - gene_pool->flatten_result_set[winner_anti_acceptance == 0 ? second_competitor : third_competitor]) / selection_param->selection_temp_param);
        acceptance[1] = 1 - acceptance[0];
        int winner_main_competitor;
        roulette_wheel(acceptance, 2, 1, &winner_main_competitor);

        // If the main competitor wins he goes through, if not it depends on the outcome of the first competition
        if (winner_main_competitor == 0) {
            gene_pool->selected_indexes[i] = main_competitor;
        }
        else {
            gene_pool->selected_indexes[i] = winner_anti_acceptance == 0 ? second_competitor : third_competitor;
        }
    }
}




void process_selection(gene_pool_t* gene_pool, selection_param_t* selection_param) {
    
    // Check if the distributions are up to date
    if (current_prob_param != selection_param->selection_prob_param || prob_distr[0] == -1) { // Change or not initialized
        compute_distr(gene_pool, selection_param);
	}
	if (current_temp_param != selection_param->selection_temp_param || boltzmann_distr[0] == -1) {
		compute_boltzmann_distr(gene_pool, selection_param);
	}

    // Select individuals
	if (selection_param->selection_method == selection_method_roulette) {
		roulette_selection(gene_pool, selection_param);
	}
	else if (selection_param->selection_method == selection_method_rank_tournament) {
		tournament_selection(gene_pool, selection_param);
	}
	else if (selection_param->selection_method == selection_method_rank) {
		rank_selection(gene_pool, selection_param);
	}
	else if (selection_param->selection_method == selection_method_rank_space) {
        space_selection(gene_pool, selection_param);
	}
	else if (selection_param->selection_method == selection_method_boltzmann) {
		boltzmann_selection(gene_pool, selection_param);
	}
	else {
		printf("Error: selection_method is not 0, 1, 2, 3 or 4\n");
	}
}