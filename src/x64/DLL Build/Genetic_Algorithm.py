from audioop import cross
import ctypes 

from Structs import *
# class ConfigGA(ctypes.Structure):
#     _fields_ = [
#         ("selection_param", SelectionParam),
#         ("flatten_param", FlattenParam),
#         ("crossover_param", CrossoverParam),
#         ("mutation_param", MutationParam),
#         ("fx_param", FxParam)
#     ]
if __name__ == "__main__":
    # $(OutDir)$(TargetName)$(TargetExt)
    
    cfg = config_ga_s()
    
    cfg.selection_param = selection_param_s(selection_method=0,
                                            selection_div_param=0.0,
                                            selection_prob_param=0.0,
                                            selection_temp_param=10.0,
                                            selection_tournament_size=0)
    
    cfg.fx_param = fx_param_s(lower=[0.0, 0.0],
                              fx_method=1,
                              fx_optim_mode=0,
                              upper=[3.0, 3.0])
    
    cfg.crossover_param = crossover_param_s(crossover_method=6,
                                            crossover_prob=0.5)
    
    cfg.flatten_param = flatten_param_s(flatten_method=0,
                                        flatten_bias=0.0,
                                        flatten_factor=1.0,
                                        flatten_optim_mode=5)
    
    cfg.mutation_param = mutation_param_s(mutation_method=0,
                                          mutation_prob=0.5,
                                          mutation_rate=6)

    rtime_param = runtime_param_s(genes=2, max_iterations=10000,
                                         individuals=32, elitism=2,
                                         convergence_threshold=1e-8,
                                         convergence_window=1000,
                                         fully_qualified_basename="C:\\temp\\GApython".encode('ansi')
                                         )


    

    dll = ctypes.CDLL(r"C:\Users\vanei\source\repos\Genetic Algorithm - C Branch\src\x64\DLL Build\Genetic Algrotihm.dll")
    geneticalgorithm = dll.Genetic_Algorithm
    print(geneticalgorithm)
    
    geneticalgorithm.argtypes = [zz_config_ga_s__, zz_runtime_param_s__] 
    for i in range(0, 10):
        print(i)
        geneticalgorithm(cfg.cType(), rtime_param.cType())

        print("result")