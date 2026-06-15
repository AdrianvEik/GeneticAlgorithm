# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys
from pathlib import Path

from clang.cindex import Config
from hawkmoth.util import readthedocs

native_libclang = Path(sys.prefix) / 'Lib' / 'site-packages' / 'clang' / 'native' / 'libclang.dll'
if native_libclang.exists():
    Config.set_library_file(str(native_libclang))

readthedocs.clang_setup()

hawkmoth_root = os.path.abspath('../..')

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'GeneticAlgorithm'
copyright = '2026, Adrian van Eik'
author = 'Adrian van Eik'
release = '1.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.coverage',
    'sphinx.ext.autosectionlabel',
    'sphinx.ext.intersphinx',
    'hawkmoth',
    'myst_parser',
]

myst_enable_extensions = [
    'colon_fence',
]

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_rtd_theme"
