
import sys
import matplotlib.pyplot as plt
import numpy as np

# fix for this to work trough import 
sys.path.append('../')

from Logger.logging import dataset, paramset

class Plot:
    def __init__(self, data: dataset, config: paramset ) -> None:
        self.data = data
        self.config = config
        
        self.iterations = dataset.iterations
        self.resultset = dataset.resultset
        self.popparamdouble = dataset.popparamdouble
        
        
    def plot_fitness(self):
        pass
        
    def boxplot_population(self):
        pass

    def plot_improvement(self):
        pass
    
    def plot_on_test_function(self):
        pass
