
from dataclasses import dataclass
import json
import struct
from typing import List
from weakref import ProxyType
import numpy as np
 
@dataclass
class runtime_param_s:
    max_iterations: int = 0
    convergence_window: int = 0
    convergence_threshold: float = 0.0
    
@dataclass
class gene_pool_param_s:
    genes: int = 0
    individuals: int = 0
    elitism: int = 0
    
@dataclass
class selection_param_s:
    selection_method: int = 0
    selection_div_param: float = 0.0
    selection_prob_param: float = 0.0
    selection_temp_param: float = 0.0
    selection_tournament_size: int = 0
    
@dataclass
class flatten_param_s:
    flatten_method: int = 0
    flatten_factor: float = 0.0
    flatten_bias: float = 0.0
    flatten_optim_mode: int = 0

@dataclass
class crossover_param_s:
    crossover_method: int = 0
    crossover_prob: float = 0.0

@dataclass
class mutate_param_s:
    mutation_method: int = 0
    mutation_prob: float = 0.0
    mutation_rate: float = 0.0
    
@dataclass
class fx_param_s:
    fx_method: int = 0
    fx_optim_mode: float = 0.0
    fx_lower: List[float] = None
    fx_upper: List[float] = None


class paramset:
    def __init__(self, fully_qualified_basename: str):
        conifg_dict: dict = json.load(open(fully_qualified_basename, mode="r"))
        self.runtime_param = runtime_param_s(**conifg_dict["runtime_param"])
        self.gene_pool_param = gene_pool_param_s(**conifg_dict["gene_pool_param"])
        self.selection_param = selection_param_s(**conifg_dict["selection_param"])
        self.flatten_param = flatten_param_s(**conifg_dict["flatten_param"])
        self.crossover_param = crossover_param_s(**conifg_dict["crossover_param"])
        self.mutate_param = mutate_param_s(**conifg_dict["mutation_param"])
        self.fx_param = fx_param_s(**conifg_dict["fx_param"])


class dataset:
    def __init__(self, fully_qualified_basename) -> None:
        self.paramset = paramset(fully_qualified_basename+".json")
        self.fqb = fully_qualified_basename
        
        self.iterations = np.array([])
        self.resultset = np.array([])
        self.popparamdouble = np.array([])
        
    def read_data(self):
        
        sizeof_iteration = 4
        sizeof_reseultset = 8 * self.paramset.gene_pool_param.individuals
        sizeof_popparamdouble = 8 * self.paramset.gene_pool_param.genes * self.paramset.gene_pool_param.individuals
        
        file = open(self.fqb + ".bin", mode="rb")
        
        data = file.read()
        
        current_byte = 0
        
        while current_byte < len(data):
            self.iterations = np.append(self.iterations, struct.unpack("i", data[current_byte:current_byte+sizeof_iteration]))
            current_byte += sizeof_iteration
            
            self.resultset = np.append(self.resultset, struct.unpack("d"*self.paramset.gene_pool_param.individuals, data[current_byte:current_byte+sizeof_reseultset]))
            current_byte += sizeof_reseultset
            
            self.popparamdouble = np.append(self.popparamdouble, struct.unpack("d"*self.paramset.gene_pool_param.genes*self.paramset.gene_pool_param.individuals, data[current_byte:current_byte+sizeof_popparamdouble]))
            current_byte += sizeof_popparamdouble
            



            
if __name__ == "__main__":
    ds = dataset("C:\\temp\\GApython")
    ds.read_data()