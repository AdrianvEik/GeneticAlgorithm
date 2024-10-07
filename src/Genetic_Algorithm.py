import ctypes 

from Structs import *

class GeneticAlgorithm:
    def __init__(self, experiment_name: str) -> None:
        self.cfg = config_ga_s()
        self.default_values_cfg()

        self.rtime_param = runtime_param_s(genes=2, max_iterations=10000,
                                           individuals=32, elitism=4,
                                        convergence_threshold=1e-8,
                                        convergence_window=1000,
                                        fully_qualified_basename=f"C:\\temp\\GA_{experiment_name}".encode('ansi'),
                                        task_count=16
                                        )
        
        dll = ctypes.CDLL(r"D:\School\Libraries\GeneticAlgorithm\src\x64\DLL Build\Genetic Algrotihm.dll")
        self.geneticalgorithm = dll.Genetic_Algorithm
        self.geneticalgorithm.argtypes = [zz_config_ga_s__, zz_runtime_param_s__] 
        
    def default_values_cfg(self):
        self.cfg.selection_param = selection_param_s(selection_method=0,
                                        selection_div_param=0.0,
                                        selection_prob_param=0.0,
                                        selection_temp_param=10.0,
                                        selection_tournament_size=0)
        
        self.cfg.fx_param = fx_param_s(fx_method=1,
                                        fx_optim_mode=0)

        self.cfg.pop_param = pop_param_s(sampling_type = 0,
                                         sigma = 1,
                                         lower=[0, 0],
                                         upper=[0, 0])
        
        self.cfg.crossover_param = crossover_param_s(crossover_method=6,
                                                    crossover_prob=0.5)
        
        self.cfg.flatten_param = flatten_param_s(flatten_method=0,
                                                flatten_bias=0.0,
                                                flatten_factor=1.0,
                                                flatten_optim_mode=5)
        
        self.cfg.mutation_param = mutation_param_s(mutation_method=0,
                                                    mutation_prob=0.5,
                                                    mutation_rate=4)

    def run(self):
        self.geneticalgorithm(self.cfg.cType(), self.rtime_param.cType())
        return None


if __name__ == "__main__":
    ga = GeneticAlgorithm("exp")
    ga.run()