# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
import sys
from pathlib import Path

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

# These are automatically set when the docs are generated using CMake.
project = 'Mobile sACN'
copyright = 'year, author'
author = 'author'
release = 'dev'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
sys.path.append(str(Path('./_ext').absolute()))
extensions = [
    'sphinx-ext-button',
    'sphinx-ext-sass',
]
sass_files = [
    '_style/style.scss'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

primary_domain = None

nitpicky = True

defs = []

# Substitutions
substitutions = {}
defs.extend(['.. |{}| {}'.format(k, v) for k, v in substitutions.items()])
del substitutions

rst_prolog = '\n\n'.join(defs)
del defs

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'alabaster'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_logo = 'logo.svg'
html_favicon = '../resources/logo.ico'
html_copy_source = False
html_show_sphinx = False
html_permalinks = False

# -- Options for LaTeX output -------------------------------------------------

latex_engine = 'xelatex'
latex_logo = '../resources/logo.svg'
latex_show_urls = 'inline'

# -- Options for QtHelp output -------------------------------------------------

# These are automatically set when the docs are generated using CMake.
qthelp_basename = 'mobilesacn'
qthelp_namespace = 'org.dankeenan.mobilesacn'
