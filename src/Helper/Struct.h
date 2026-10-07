
#ifndef STRUCT_H
#define STRUCT_H

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * Core Data Model
 * ===============
 *
 * These structs define the shared vocabulary used by the C genetic algorithm.
 * The runtime entry point receives :c:type:`config_ga_t` and
 * :c:type:`runtime_param_t`, solver threads allocate a :c:type:`gene_pool_t`,
 * and each pipeline stage consumes its own smaller parameter struct.
 */

/**
 * Shared state for one genetic algorithm population.
 *
 * ``gene_pool_t`` owns the memory blocks that operators mutate during a run:
 * binary chromosomes, decoded double parameters, fitness values, sorted
 * indexes, selection scratch space, crossover buffers, and fitness-task
 * completion flags. Allocate it with :c:func:`init_gene_pool` and release it
 * with :c:func:`free_gene_pool`.
 */
struct gene_pool_s {
	/** Aligned allocation that owns all population buffers. */
	char* gene_pool_memory_ptr;
	/** Binary chromosome matrix, indexed as ``[individual][gene_block]``. */
	uint32_t** pop_param_bin;
	/** Staging matrix used by crossover before offspring replace parents. */
	uint32_t** pop_param_bin_cross_buffer;
	/** Decoded floating-point parameters for objective functions. */
	double** pop_param_double;
	/** Canonical measured scores, signed for min/max; physical individual order. */
	double* pop_result_set;
	/** Mandatory [0,1] normalization of canonical scores, in physical order. */
	double* normalized_result_set;
	/** Finite nonnegative selection weights derived from normalized scores. */
	double* flatten_result_set;
	/** Scratch array used by selection probability calculations. */
	double* selection_temp;
	/** Per-rank mutation rates for the task currently using this pool. */
	double* mutation_rate;
	/** Source indexes selected as parents for the next generation. */
	uint32_t* selected_indexes;
	/** Indexes sorted by ``pop_result_set`` from worst to best. */
	uint32_t* sorted_indexes;
	/** Temporary workspace for indexed sorting. */
    uint32_t* sorted_indexes_temp;
	/** Per-physical-individual duplicate markers; never stored in measured scores. */
	uint32_t* duplicate_flags;
	/** Physical slots scheduled for reseeding; independent of parent selection. */
	uint32_t* reseed_indexes;
	/** Number of active entries in reseed_indexes, reset at each task boundary. */
	uint32_t reseed_count;
	/** Per-individual completion flags for parallel fitness subtasks. */
	_Atomic(uint32_t)* fx_ready;
	/** Number of genes in each individual. */
	uint32_t genes;
	/** Number of individuals in the population. */
	uint32_t individuals;
	/** Number of best individuals protected from replacement. */
	uint32_t elitism;
	/** Current generation counter for the active solver task. */
	uint32_t iteration_number;
	/** Number of bits used to represent one gene. */
	uint32_t gene_mem_size;
    /** Aligned byte count used by one binary individual. */
    uint32_t individual_mem_size;
};


static const int pop_uniform = 0;
static const int pop_normal = 1;

/**
 * Population initialization settings.
 *
 * The population stage uses these bounds and sampling controls when
 * :c:func:`fill_pop` seeds the binary population before the first iteration.
 */
struct population_param_s {
	/** Population sampler, such as :c:var:`pop_uniform` or :c:var:`pop_normal`. */
	uint32_t sampling_type;
	/** Spread parameter reserved for distribution-based samplers. */
	uint32_t sigma;
	/** Per-gene lower search bounds for double-valued objectives. */
	double* lower;
	/** Per-gene upper search bounds for double-valued objectives. */
	double* upper;
	/** Number of weakest individuals to reseed after sorting. */
	uint32_t reseed_bottom_N;
};

static const int selection_method_roulette = 0;
static const int selection_method_rank_tournament = 1;
static const int selection_method_rank = 2;
static const int selection_method_rank_space = 3;
static const int selection_method_boltzmann = 4;
static const int selection_method_logistic_pairwise = 5;
static const int selection_method_boltzmann_strict = 6;
static const int selection_method_boltzmann_relaxed = 7;

/**
 * Parent-selection settings.
 *
 * ``selection_method`` chooses the selection routine used by
 * :c:func:`process_selection`; the remaining fields tune the probability,
 * tournament, rank, or Boltzmann distributions used by those routines.
 */
struct selection_param_s {
	/** Selection algorithm identifier. */
	int selection_method;
	/** Rank-space mixture fraction lambda in [0,1]. */
	double selection_div_param;
	/** Geometric rank pressure p in [0,1]; 0 is uniform, 1 selects the best group. */
	double selection_prob_param;
	/** Positive finite temperature; tournament differences use normalized units. */
	double selection_temp_param;
	/** Number of competitors sampled for tournament selection. */
	uint32_t selection_tournament_size;
    /** Distribution source for rank-based selection variants. */
    uint32_t selection_rank_distr;
    /** Normalized fitness-class separation for three-candidate Boltzmann, [0,1]. */
    double selection_boltzmann_threshold;
};

static const int flatten_method_linear = 0;
static const int flatten_method_exponential = 1;
static const int flatten_method_logarithmic = 2;
static const int flatten_method_normalized = 3;
static const int flatten_method_sigmoid = 4;
static const int flatten_method_none = 5;

/**
 * Fitness flattening settings.
 *
 * Flattening transforms ``normalized_result_set`` into ``flatten_result_set`` before
 * selection so selection pressure can be softened or emphasized without
 * changing the underlying objective function.
 */
struct flatten_param_s {
	/** Flattening method identifier. */
	int flatten_method;
	/** Finite nonnegative slope (linear) or gain/curvature (exp/log/sigmoid). */
	double flatten_alpha;
	/** Linear baseline >=0; sigmoid center in [0,1]; unused by other methods. */
	double flatten_beta;
};

static const int crossover_method_single_point = 0;
static const int crossover_method_two_point = 1;
static const int crossover_method_uniform = 2;
static const int crossover_method_complete = 3;


/**
 * Crossover settings.
 *
 * ``crossover_method`` selects the recombination strategy used by
 * :c:func:`process_crossover`; ``crossover_prob`` controls how often selected
 * pairs are crossed instead of copied.
 */
struct crossover_param_s {
	/** Crossover method identifier. */
	int crossover_method;
	/** Probability that a selected parent pair is recombined. */
	double crossover_prob;
};

 //TODO: per gene mutation probability and mutation pressure (rank dependant)

// complete bit level random 
static const int mutation_method_bit_level = 0;
// bounded gene level random
static const int mutation_method_gene_level = 1;
 
 /**
 * Mutation settings.
 *
 * The mutable per-rank mutation-rate array belongs to :c:type:`gene_pool_t`.
 * This structure contains only the immutable settings used to initialize and
 * update those rates for each task.
 */
struct mutation_param_s {
	/** Mutation method identifier. */
	int mutation_method;
	/** Base mutation probability used by mutation dispatchers. */
	double mutation_prob;
	/** Mutation rate assigned to every population rank when a task starts. */
	double initial_mutation_rate;
    /** Rank-dependent slope applied by adaptive mutation updates. */
    double mutation_slope;
    //double mutation_alpha; // DEFAULT = 1
    //double mutation_beta; // DEFAULT = 0
};

static const int fx_method_pointer = -1;
static const int fx_method_Styblinski_Tang = 0;
static const int fx_method_Wheelers_Ridge = 1;
static const int fx_method_Genetic_Algorithm = 2;
static const int fx_method_Ackley = 3;
static const int fx_method_Griewank = 4;
static const int fx_method_Langermann = 5;
static const int fx_method_Levy = 6;
static const int fx_method_Rastrigin = 7;
static const int fx_method_Schwefel = 8;

static const int fx_data_type_double = 1;
static const int fx_data_type_int = 2;

static const int fx_optim_mode_minimize = -1;
static const int fx_optim_mode_maximize = 1;

/**
 * User-supplied objective callback.
 *
 * :param paramset: Pointer to one decoded individual. The concrete type is
 *     selected by ``fx_data_type`` and is either ``double*`` or ``uint32_t*``.
 * :param genes: Number of genes in ``paramset``.
 * :returns: Objective value before the configured optimization-mode sign is
 *     applied.
 */
typedef double (*fx_ptr_generic)(void*, uint32_t);

/**
 * Fitness-function settings.
 *
 * The fitness stage uses these values to choose a built-in objective, nested
 * GA objective, or user callback, and to decide whether the decoded parameter
 * buffer is interpreted as doubles or integers. Ackley, Griewank, Langermann,
 * Levy, Rastrigin and Schwefel built-ins require doubles and use minimization.
 * They use the defaults documented in ``Function/Benchmarks.h``; Langermann
 * requires two genes. Set population bounds explicitly for the chosen domain.
 * For custom constants, use a callback wrapper around the parameterized API.
 */
struct fx_param_s {
	/** Objective selector, including built-ins and callback mode. */
	int fx_method;
	/** Optimization direction, either minimize or maximize. */
	int fx_optim_mode;
    /** Decoded parameter type passed to the objective. */
    int fx_data_type;
	/** Optional callback used when ``fx_method`` is :c:var:`fx_method_pointer`. */
	fx_ptr_generic fx_function;
};

/**
 * Adaptive optimizer controls.
 *
 * These values bound mutation-rate adaptation and convergence detection inside
 * :c:func:`adapt_param`.
 */
struct optimizer_param_s {
	/** Minimum mutation rate assigned by adaptive updates. */
	double min_mutations;
	/** Maximum mutation rate assigned by adaptive updates. */
	double max_mutations;
	/** Scaling factor reserved for mutation adaptation. */
	double mutation_factor;
	/** Smallest result delta considered meaningful progress. */
	double convergence_threshold;
	/** Window size used for moving-average convergence state. */
	uint32_t convergence_moving_window_size;
	/** Hard iteration cap for one solver task. */
	uint32_t max_iterations;
	/** Number of stable iterations required before convergence. */
	uint32_t convergence_window;
};

/**
 * Complete genetic algorithm configuration.
 *
 * This struct groups the population, selection, flattening, crossover,
 * mutation, objective, and optimizer settings passed into
 * :c:func:`Genetic_Algorithm`.
 */
struct config_ga_s {
	/** Population seeding configuration. */
	struct population_param_s population_param;
	/** Parent-selection configuration. */
	struct selection_param_s selection_param;
	/** Fitness-flattening configuration. */
	struct flatten_param_s flatten_param;
	/** Crossover configuration. */
	struct crossover_param_s crossover_param;
	/** Mutation configuration. */
	struct mutation_param_s mutation_param;
	/** Objective-function configuration. */
	struct fx_param_s fx_param;
    /** Adaptive optimizer configuration. */
    struct optimizer_param_s optimizer_param;
};

/**
 * Runtime logging and export settings.
 *
 * The logging thread uses these values to size result buffers, choose CSV/bin
 * output, configure console progress, and optionally dump a JSON
 * configuration file before a run starts.
 */
struct logging_param_s {
	/** Output basename used for CSV, binary, and JSON files. */
	char* fully_qualified_basename;
    /** Number of sorted individuals exported for ordinary log records. */
    uint32_t top_n_export;
    /** Iteration interval for intermediate exports; ``0`` means final only. */
    uint32_t export_interval;
    /** Non-zero to include adaptive/config values in result rows. */
    int include_config;
    /** Non-zero to write CSV output. */
    int write_csv;
    /** Non-zero to write binary output. */
    int write_bin;
    /** Number of integer config fields reserved in binary rows. */
    uint32_t config_int_count;
    /** Number of double config fields reserved in binary rows. */
    uint32_t config_double_count;
    /** Result queue capacity. */
    uint32_t queue_size;
    /** Non-zero to export a JSON configuration snapshot. */
    int write_config;
    /** Non-zero to render terminal progress output. */
    int console_enabled;
};

/**
 * Runtime execution settings.
 *
 * Runtime parameters define the population dimensions, worker counts, task
 * partitioning, bit width, random seeding, and logging behavior for one
 * :c:func:`Genetic_Algorithm` invocation.
 */
struct runtime_param_s {
	/** Optional observer after each process_pop/adapt_param pair, on solver threads.
	 * seconds measures that pair with QueryPerformanceCounter, excluding observer
	 * and logging work. Pool is borrowed and must not be modified. Callbacks must
	 * synchronize their context when using multiple solver threads. NULL disables
	 * instrumentation. iteration_number is zero-based. */
	void (*generation_observer)(const struct gene_pool_s* pool, uint32_t task_id,
	                            double seconds, void* context);
	void* generation_observer_context;
	/** Number of genes per individual. */
	uint32_t genes;
	/** Number of individuals per population. */
	uint32_t individuals;
	/** Number of best individuals preserved each generation. */
	uint32_t elitism;
	/** Requested number of solver tasks. */
	uint32_t task_count_solver;
	/** Number of solver worker threads. */
	uint32_t thread_count_solver;
	/** Batch size; must be 0 with thread_count_fx == 0, or positive with fitness workers. */
	uint32_t task_size_fx;
    /** Fitness worker count; both fitness settings must be 0 for inline evaluation, or both positive. */
    uint32_t thread_count_fx;
    /** Non-zero to split the search space into zones. */
    int zone_enable;
    /** Number of bits used to encode one gene. */
    uint32_t gene_mem_size;
    /** Base RNG seed; tasks use base + task_id modulo 2^32. ``0`` requests automatic seeding per task. */
    uint32_t random_seed;
    /** Logging and progress-display configuration. */
    struct logging_param_s logging_param;
};	

typedef struct gene_pool_s gene_pool_t;
typedef struct selection_param_s selection_param_t;
typedef struct flatten_param_s flatten_param_t;
typedef struct crossover_param_s crossover_param_t;
typedef struct mutation_param_s mutation_param_t;
typedef struct fx_param_s fx_param_t;
typedef struct population_param_s population_param_t;
typedef struct config_ga_s config_ga_t;
typedef struct runtime_param_s runtime_param_t;
typedef struct optimizer_param_s optimizer_param_t;
typedef struct logging_param_s logging_param_t;

/**
 * Build the default runtime configuration.
 *
 * :returns: A ``runtime_param_t`` with conservative defaults for population
 *     size, thread counts, bit width, zone splitting, random seeding, and
 *     logging.
 */
runtime_param_t default_runtime_param();

/**
 * Build the default genetic algorithm configuration for a runtime.
 *
 * This allocates per-gene population bounds, so callers must release the
 * returned struct with :c:func:`free_config_ga`. Mutable per-rank mutation
 * rates are allocated separately with each gene pool.
 *
 * :param runtime_param: Runtime dimensions used to size configuration arrays.
 * :returns: A ``config_ga_t`` initialized with built-in defaults.
 */
config_ga_t default_config(runtime_param_t runtime_param);

/**
 * Release heap allocations owned by a configuration.
 *
 * :param config_ga: Configuration returned by :c:func:`default_config`.
 */
void free_config_ga(config_ga_t* config_ga);

/**
 * Validate configuration and runtime values before a run starts.
 *
 * Exits through the project error-handling macros when core invariants are
 * violated, such as elitism being larger than the population.
 *
 * :param config_ga: Genetic algorithm configuration to validate.
 * :param runtime_param: Runtime settings to validate against the config.
 */
void verify_input_parameters(config_ga_t config_ga, runtime_param_t runtime_param);
#endif // STRUCT_H
