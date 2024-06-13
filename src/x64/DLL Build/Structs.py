import ctypes


# class zz_gene_pool_s__(ctypes.Structure):
#     _fields_ = [
#         ("pop_result_set", ctypes.c_double_p),
#         ("flatten_result_set", ctypes.c_double_p),
#         ("selected_indexes", ctypes.c_int_p),
#         ("sorted_indexes", ctypes.c_int_p),
#         ("genes", ctypes.c_int),
#         ("individuals", ctypes.c_int),
#         ("elitism", ctypes.c_int),
#         ("iteration_number", ctypes.c_int)
#     ]


class zz_selection_param_s__(ctypes.Structure):
    _fields_ = [
        ("selection_method", ctypes.c_int),
        ("selection_div_param", ctypes.c_double),
        ("selection_prob_param", ctypes.c_double),
        ("selection_temp_param", ctypes.c_double),
        ("selection_tournament_size", ctypes.c_int)
    ]


class zz_flatten_param_s__(ctypes.Structure):
    _fields_ = [
        ("flatten_method", ctypes.c_int),
        ("flatten_factor", ctypes.c_double),
        ("flatten_bias", ctypes.c_double),
        ("flatten_optim_mode", ctypes.c_int)
    ]


class zz_crossover_param_s__(ctypes.Structure):
    _fields_ = [
        ("crossover_method", ctypes.c_int),
        ("crossover_prob", ctypes.c_double)
    ]


class zz_mutation_param_s__(ctypes.Structure):
    _fields_ = [
        ("mutation_method", ctypes.c_int),
        ("mutation_prob", ctypes.c_double),
        ("mutation_rate", ctypes.c_int)
    ]


class zz_fx_param_s__(ctypes.Structure):
    _fields_ = [
        ("fx_method", ctypes.c_int),
        ("fx_optim_mode", ctypes.c_int),
        ("bin2double_factor", ctypes.c_double),
        ("bin2double_bias", ctypes.c_double)
    ]


class zz_config_ga_s__(ctypes.Structure):
    _fields_ = [
        ("selection_param", zz_selection_param_s__),
        ("flatten_param", zz_flatten_param_s__),
        ("crossover_param", zz_crossover_param_s__),
        ("mutation_param", zz_mutation_param_s__),
        ("fx_param", zz_fx_param_s__)
    ]


class zz_runtime_param_s__(ctypes.Structure):
    _fields_ = [
        ("fully_qualified_basename", ctypes.c_char_p),
        ("max_iterations", ctypes.c_int),
        ("convergence_threshold", ctypes.c_double),
        ("convergence_window", ctypes.c_int),
        ("genes", ctypes.c_int),
        ("individuals", ctypes.c_int),
        ("elitism", ctypes.c_int)
    ]


# class gene_pool_s:
#     pop_result_set = ctypes.c_double_p()
#     flatten_result_set = ctypes.c_double_p()
#     selected_indexes = ctypes.c_int_p()
#     sorted_indexes = ctypes.c_int_p()
#     genes = 0
#     individuals = 0
#     elitism = 0
#     iteration_number = 0
#     def __init__(
#             self,
#             pop_result_set: ctypes.c_double_p = ctypes.c_double_p(),
#             flatten_result_set: ctypes.c_double_p = ctypes.c_double_p(),
#             selected_indexes: ctypes.c_int_p = ctypes.c_int_p(),
#             sorted_indexes: ctypes.c_int_p = ctypes.c_int_p(),
#             genes: int = 0,
#             individuals: int = 0,
#             elitism: int = 0,
#             iteration_number: int = 0
#         ):
#         self.pop_result_set = pop_result_set
#         self.flatten_result_set = flatten_result_set
#         self.selected_indexes = selected_indexes
#         self.sorted_indexes = sorted_indexes
#         self.genes = genes
#         self.individuals = individuals
#         self.elitism = elitism
#         self.iteration_number = iteration_number
#     def cType(self):
#         return zz_gene_pool_s__(
#             ctypes.c_double_p(self.pop_result_set),
#             ctypes.c_double_p(self.flatten_result_set),
#             ctypes.c_int_p(self.selected_indexes),
#             ctypes.c_int_p(self.sorted_indexes),
#             ctypes.c_int(self.genes),
#             ctypes.c_int(self.individuals),
#             ctypes.c_int(self.elitism),
#             ctypes.c_int(self.iteration_number)
#         )


class selection_param_s:
    selection_method = 0
    selection_div_param = 0.0
    selection_prob_param = 0.0
    selection_temp_param = 0.0
    selection_tournament_size = 0
    def __init__(
            self,
            selection_method: int = 0,
            selection_div_param: float = 0.0,
            selection_prob_param: float = 0.0,
            selection_temp_param: float = 0.0,
            selection_tournament_size: int = 0
        ):
        self.selection_method = selection_method
        self.selection_div_param = selection_div_param
        self.selection_prob_param = selection_prob_param
        self.selection_temp_param = selection_temp_param
        self.selection_tournament_size = selection_tournament_size
    def cType(self):
        return zz_selection_param_s__(
            ctypes.c_int(self.selection_method),
            ctypes.c_double(self.selection_div_param),
            ctypes.c_double(self.selection_prob_param),
            ctypes.c_double(self.selection_temp_param),
            ctypes.c_int(self.selection_tournament_size)
        )


class flatten_param_s:
    flatten_method = 0
    flatten_factor = 0.0
    flatten_bias = 0.0
    flatten_optim_mode = 0
    def __init__(
            self,
            flatten_method: int = 0,
            flatten_factor: float = 0.0,
            flatten_bias: float = 0.0,
            flatten_optim_mode: int = 0
        ):
        self.flatten_method = flatten_method
        self.flatten_factor = flatten_factor
        self.flatten_bias = flatten_bias
        self.flatten_optim_mode = flatten_optim_mode
    def cType(self):
        return zz_flatten_param_s__(
            ctypes.c_int(self.flatten_method),
            ctypes.c_double(self.flatten_factor),
            ctypes.c_double(self.flatten_bias),
            ctypes.c_int(self.flatten_optim_mode)
        )


class crossover_param_s:
    crossover_method = 0
    crossover_prob = 0.0
    def __init__(
            self,
            crossover_method: int = 0,
            crossover_prob: float = 0.0
        ):
        self.crossover_method = crossover_method
        self.crossover_prob = crossover_prob
    def cType(self):
        return zz_crossover_param_s__(
            ctypes.c_int(self.crossover_method),
            ctypes.c_double(self.crossover_prob)
        )


class mutation_param_s:
    mutation_method = 0
    mutation_prob = 0.0
    mutation_rate = 0
    def __init__(
            self,
            mutation_method: int = 0,
            mutation_prob: float = 0.0,
            mutation_rate: int = 0
        ):
        self.mutation_method = mutation_method
        self.mutation_prob = mutation_prob
        self.mutation_rate = mutation_rate
    def cType(self):
        return zz_mutation_param_s__(
            ctypes.c_int(self.mutation_method),
            ctypes.c_double(self.mutation_prob),
            ctypes.c_int(self.mutation_rate)
        )


class fx_param_s:
    fx_method = 0
    fx_optim_mode = 0
    bin2double_factor = 0.0
    bin2double_bias = 0.0
    def __init__(
            self,
            fx_method: int = 0,
            fx_optim_mode: int = 0,
            bin2double_factor: float = 0.0,
            bin2double_bias: float = 0.0
        ):
        self.fx_method = fx_method
        self.fx_optim_mode = fx_optim_mode
        self.bin2double_factor = bin2double_factor
        self.bin2double_bias = bin2double_bias
    def cType(self):
        return zz_fx_param_s__(
            ctypes.c_int(self.fx_method),
            ctypes.c_int(self.fx_optim_mode),
            ctypes.c_double(self.bin2double_factor),
            ctypes.c_double(self.bin2double_bias)
        )


class config_ga_s:
    selection_param = selection_param_s()
    flatten_param = flatten_param_s()
    crossover_param = crossover_param_s()
    mutation_param = mutation_param_s()
    fx_param = fx_param_s()
    def __init__(
            self,
            selection_param: selection_param_s = selection_param_s(),
            flatten_param: flatten_param_s = flatten_param_s(),
            crossover_param: crossover_param_s = crossover_param_s(),
            mutation_param: mutation_param_s = mutation_param_s(),
            fx_param: fx_param_s = fx_param_s()
        ):
        self.selection_param = selection_param
        self.flatten_param = flatten_param
        self.crossover_param = crossover_param
        self.mutation_param = mutation_param
        self.fx_param = fx_param
    def cType(self):
        return zz_config_ga_s__(
            self.selection_param.cType(),
            self.flatten_param.cType(),
            self.crossover_param.cType(),
            self.mutation_param.cType(),
            self.fx_param.cType()
        )


class runtime_param_s:
    fully_qualified_basename: str = ""
    max_iterations = 0
    convergence_threshold = 0.0
    convergence_window = 0
    genes = 0
    individuals = 0
    elitism = 0
    def __init__(
            self,
            fully_qualified_basename: str = "",
            max_iterations: int = 0,
            convergence_threshold: float = 0.0,
            convergence_window: int = 0,
            genes: int = 0,
            individuals: int = 0,
            elitism: int = 0
        ):
        self.fully_qualified_basename = fully_qualified_basename
        self.max_iterations = max_iterations
        self.convergence_threshold = convergence_threshold
        self.convergence_window = convergence_window
        self.genes = genes
        self.individuals = individuals
        self.elitism = elitism
    def cType(self):
        return zz_runtime_param_s__(
            ctypes.c_char_p(self.fully_qualified_basename),
            ctypes.c_int(self.max_iterations),
            ctypes.c_double(self.convergence_threshold),
            ctypes.c_int(self.convergence_window),
            ctypes.c_int(self.genes),
            ctypes.c_int(self.individuals),
            ctypes.c_int(self.elitism)
        )

