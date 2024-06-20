
import sys
import matplotlib.pyplot as plt
import numpy as np

from typing import Dict, Union
from numpy.typing import NDArray

# fix for this to work trough import 
sys.path.append('../')

from Logger.logging import Dataset, Paramset

class Plot:
    def __init__(self, data: Union[Dataset, Dict[str, Dataset]]) -> None:
        """
        Initialize the Plot class.

        Args:
            data (Union[Dataset, Dict[str, Dataset]]): A single Dataset object or a dictionary of Dataset objects.

        Raises:
            TypeError: If the data is not a Dataset instance or a dictionary of Dataset instances.
        """
        if isinstance(data, Dataset):
            self.datasets = {"Model": data}
        elif isinstance(data, dict):
            self.datasets = data
        else:
            raise TypeError("Data must be a Dataset instance or a dictionary of Dataset instances")

    def plot_fitness(self, show_population=True):
        """
        Plot the fitness of the best individual over all iterations.
        
        :param show_population: Whether to show the worst individual and average population fitness. Default is True.
        :type show_population: Bool
        
        """
        for label, data in self.datasets.items():
            iterations = data.iterations
            resultset = data.resultset
            plt.plot(iterations, resultset[:, -1], label=f"{label} - Best individual")

            if show_population:
                plt.plot(iterations, resultset[:, 0], label=f"{label} - Worst individual")
                plt.plot(iterations, np.mean(resultset, axis=1), label=f"{label} - Average individual")

                std = np.std(resultset, axis=1)
                plt.fill_between(iterations, np.mean(resultset, axis=1) - std, np.mean(resultset, axis=1) + std, alpha=0.2)

        plt.xlabel("Iterations")
        plt.ylabel("Fitness")
        plt.title("Fitness of the best individual over all iterations")
        plt.legend()
        plt.show()

    def boxplot_population(self):
        """
        Create a boxplot of all the genes in the population over all iterations.
        """
        for label, data in self.datasets.items():
            popparamdouble = data.popparamdouble
            config = data.paramset
            plt.boxplot(popparamdouble.reshape(-1, config.gene_pool_param.genes), positions=range(len(config.gene_pool_param.genes)), labels=[f"{label} - Gene {i+1}" for i in range(len(config.gene_pool_param.genes))])
        
        plt.xlabel("Genes")
        plt.ylabel("Value")
        plt.title("Boxplot of all the genes in the population over all iterations")
        plt.show()

    def plot_improvement(self):
        """
        Plot the improvement per iteration, showing the difference between the best and worst individual in the population,
        and the average improvement of the population and the best individual over all iterations.
        """
        for label, data in self.datasets.items():
            best_individual = data.resultset[:, -1]
            worst_individual = data.resultset[:, 0]
            avg_population = np.mean(data.resultset, axis=1)
            avg_best_individual = np.mean(best_individual)
            
            avg_population_improvement = np.diff(avg_population)
            avg_best_individual_improvement = np.diff(best_individual)
            
            iterations = data.iterations
            plt.plot(iterations[:-1], avg_population_improvement, label=f"{label} - Avg population improvement")
            plt.plot(iterations[:-1], avg_best_individual_improvement, label=f"{label} - Avg best individual improvement")

        plt.xlabel("Iterations")
        plt.ylabel("Improvement")
        plt.title("Improvement per iteration")
        plt.legend()
        plt.show()

    def plot_convergence(self):
        """
        Plot the convergence of the population over all iterations. Convergence is defined as the deviation over a window of the best value and the convergence threshold.
        """
        for label, data in self.datasets.items():
            best_individual = data.resultset[:, -1]
            convergence_window = data.paramset.runtime_param.convergence_window
            convergence_threshold = data.paramset.runtime_param.convergence_threshold
            
            delta_best = np.abs(np.diff(best_individual))
            
            convergence_ticker = 0
            start, stop = 0, 0
            for i in range(len(delta_best)):
                if delta_best[i] <= convergence_threshold:
                    convergence_ticker += 1
                else:
                    convergence_ticker = 0
                    
                if convergence_ticker > convergence_window:
                    start, stop = i - convergence_window, i
                    break

            elitism = data.paramset.gene_pool_param.elitism
            plt.plot(data.iterations[start:stop], data.resultset[start:stop, -1], label=f"{label} - Converged", c="Green")
            plt.plot(data.iterations[:start+1], data.resultset[:start+1, -1], label=f"{label} - Not converged", c="Red")

        plt.legend()
        plt.xlabel("Iterations")
        plt.ylabel("Convergence")
        plt.title("Convergence of the population over all iterations")
        plt.show()

    def plot_population_gain(self):
        """
        Plot the gain of the population over all iterations. Gain is defined as the difference between the best and worst individual.
        """
        for label, data in self.datasets.items():
            gain = data.resultset[:, -1] - data.resultset[:, 0]
            plt.plot(data.iterations, gain, label=f"{label} - Population gain")

        plt.xlabel("Iterations")
        plt.ylabel("Gain")
        plt.title("Gain of the population over all iterations")
        plt.legend()
        plt.show()

if __name__ == "__main__":
    ds = Dataset("C:\\temp\\GApython")
    ds.read_data()
    
    plot = Plot(ds)
    plot.plot_fitness()
    plot.plot_convergence()
