Current API Reference
=====================

This page documents the project-owned C API surface. The groups below use
Hawkmoth's focused directives so structs can be rendered with their documented
members, similar to class attributes.

Core Entry Point
----------------

.. c:autofunction:: Genetic_Algorithm
   :file: src/Genetic_Algorithm.h

Configuration and Runtime Types
-------------------------------

.. c:autostruct:: gene_pool_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: population_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: selection_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: flatten_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: crossover_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: mutation_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: fx_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: optimizer_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: config_ga_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: logging_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autostruct:: runtime_param_s
   :file: src/Helper/Struct.h
   :members:

.. c:autofunction:: default_runtime_param
   :file: src/Helper/Struct.h

.. c:autofunction:: default_config
   :file: src/Helper/Struct.h

.. c:autofunction:: free_config_ga
   :file: src/Helper/Struct.h

.. c:autofunction:: verify_input_parameters
   :file: src/Helper/Struct.h

Population and Operators
------------------------

.. c:autofunction:: init_gene_pool
   :file: src/Utility/pop.h

.. c:autofunction:: free_gene_pool
   :file: src/Utility/pop.h

.. c:autofunction:: fill_pop
   :file: src/Utility/pop.h

.. c:autofunction:: fill_individual_uniform
   :file: src/Utility/pop.h

.. c:autofunction:: process_fx_set
   :file: src/Function/Function.h

.. c:autofunction:: process_fx
   :file: src/Function/Function.h

.. c:autofunction:: process_pop
   :file: src/Utility/process.h

.. c:autofunction:: process_flatten
   :file: src/Utility/flatten.h

.. c:autofunction:: process_selection
   :file: src/Utility/selection.h

.. c:autofunction:: process_crossover
   :file: src/Utility/crossover.h

.. c:autofunction:: process_mutation
   :file: src/Utility/mutation.h

Benchmark Objectives
--------------------

.. c:autofunction:: Styblinski_Tang_fx
   :file: src/Function/Benchmarks.h

.. c:autofunction:: wheelers_ridge_fx
   :file: src/Function/Benchmarks.h

Select these objectives with ``fx_method_Ackley``, ``fx_method_Griewank``,
``fx_method_Langermann``, ``fx_method_Levy``, ``fx_method_Rastrigin`` or
``fx_method_Schwefel``. They require ``fx_data_type_double`` and minimize the
objective. Set the population bounds to the desired domain; selecting a method
does not change the bounds. Default Langermann requires exactly two genes.
For custom Ackley or Langermann parameters, use ``fx_method_pointer`` with a
callback wrapper and explicitly select ``fx_optim_mode_minimize``.

.. c:autofunction:: Ackley_fx
   :file: src/Function/Benchmarks.h

.. c:autofunction:: Ackley_param_fx
   :file: src/Function/Benchmarks.h

.. c:autofunction:: Griewank_fx
   :file: src/Function/Benchmarks.h

.. c:autofunction:: Langermann_fx
   :file: src/Function/Benchmarks.h

.. c:autofunction:: Langermann_param_fx
   :file: src/Function/Benchmarks.h

.. c:autofunction:: Levy_fx
   :file: src/Function/Benchmarks.h

.. c:autofunction:: Rastrigin_fx
   :file: src/Function/Benchmarks.h

.. c:autofunction:: Schwefel_fx
   :file: src/Function/Benchmarks.h

Adaptive Optimization and Randomness
------------------------------------

.. c:autostruct:: adaptive_memory_s
   :file: src/Optimisation/Optimizer.h
   :members:

.. c:autofunction:: new_adaptive_memory
   :file: src/Optimisation/Optimizer.h

.. c:autofunction:: adapt_param
   :file: src/Optimisation/Optimizer.h

.. c:autofunction:: seed_rand_threadlocal
   :file: src/Helper/rng.h

.. c:autofunction:: gen_mt_rand
   :file: src/Helper/rng.h

.. c:autofunction:: gen_mt_rand64
   :file: src/Helper/rng.h

Task Queues and Threading
-------------------------

.. c:autostruct:: task_param_s
   :file: src/Multiprocessing/mp_solver_th.h
   :members:

.. c:autostruct:: task_queue_s
   :file: src/Multiprocessing/mp_solver_th.h
   :members:

.. c:autostruct:: fx_task_param_s
   :file: src/Multiprocessing/mp_solver_th.h
   :members:

.. c:autostruct:: fx_task_queue_s
   :file: src/Multiprocessing/mp_solver_th.h
   :members:

.. c:autostruct:: thread_param_s
   :file: src/Multiprocessing/mp_solver_th.h
   :members:

.. c:autofunction:: init_task_queue
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: free_task_queue
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: init_task
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: free_task
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: add_task
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: get_task
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: stop_task_solver_threads
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: init_fx_task_queue
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: free_fx_task_queue
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: add_fx_task
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: get_fx_task
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: stop_fx_task_threads
   :file: src/Multiprocessing/mp_solver_th.h

.. c:autofunction:: compute_task_count
   :file: src/Multiprocessing/mp_task_gen.h

.. c:autofunction:: make_task_list
   :file: src/Multiprocessing/mp_task_gen.h

Logging and Progress
--------------------

.. c:autostruct:: task_result_s
   :file: src/Multiprocessing/mp_logger.h
   :members:

.. c:autostruct:: progress_s
   :file: src/Multiprocessing/mp_logger.h
   :members:

.. c:autostruct:: task_result_queue_s
   :file: src/Multiprocessing/mp_logger.h
   :members:

.. c:autostruct:: console_message_s
   :file: src/Multiprocessing/mp_progress_disp.h
   :members:

.. c:autostruct:: console_queue_s
   :file: src/Multiprocessing/mp_progress_disp.h
   :members:

.. c:autofunction:: init_task_result_queue
   :file: src/Multiprocessing/mp_logger.h

.. c:autofunction:: free_task_result_queue
   :file: src/Multiprocessing/mp_logger.h

.. c:autofunction:: init_task_result
   :file: src/Multiprocessing/mp_logger.h

.. c:autofunction:: add_result
   :file: src/Multiprocessing/mp_logger.h

.. c:autofunction:: get_result
   :file: src/Multiprocessing/mp_logger.h

.. c:autofunction:: stop_result_logger
   :file: src/Multiprocessing/mp_logger.h

.. c:autofunction:: init_console_queue
   :file: src/Multiprocessing/mp_progress_disp.h

.. c:autofunction:: free_console_queue
   :file: src/Multiprocessing/mp_progress_disp.h

.. c:autofunction:: get_from_console_queue
   :file: src/Multiprocessing/mp_progress_disp.h

.. c:autofunction:: con_printf
   :file: src/Multiprocessing/mp_progress_disp.h

.. c:autofunction:: copy_task_result
   :file: src/Logger/logging.h

.. c:autofunction:: report_task
   :file: src/Logger/logging.h

.. c:autofunction:: write_file_buffer
   :file: src/Logger/logging.h

.. c:autofunction:: open_file
   :file: src/Logger/logging.h

.. c:autofunction:: close_file
   :file: src/Logger/logging.h

.. c:autofunction:: write_config
   :file: src/Logger/dump_config.h

.. c:autofunction:: display_progress
   :file: src/Logger/progress_display.h

.. c:autofunction:: display_console_message
   :file: src/Logger/progress_display.h
