# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
import sys
import json
from datetime import date
from pathlib import Path

# -- Build info --------------------------------------------------------------
with open('build_info.json') as f:
    build_info = json.load(f)


# -- Project information -----------------------------------------------------

project = build_info['name']
copyright = '{year}, {author}'.format(year=date.today().year, author=build_info['author'])
author = build_info['author']
# The full version, including alpha/beta/rc tags
version = build_info['version']
release = version

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
sass_include_paths = [
    str(Path('../mobile_sacn_webui/node_modules').absolute())
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
html_theme = 'sphinx_book_theme'
html_static_path = ['_static']
html_title = '{name} v{version}'.format(name=build_info['display_name'], version=release)
html_logo = 'logo.svg'
html_favicon = '../resources/logo.ico'
html_copy_source = False
html_domain_indices = False
html_show_sourcelink = False
html_show_sphinx = False
html_show_copyright = False
html_theme_options = {
    'navigation_with_keys': False,
    'repository_url': 'https://github.com/danielskeenan/mobile_sacn',
    'use_source_button': False,
    'use_edit_page_button': False,
    'use_repository_button': True,
    'use_issues_button': True,
    'use_download_button': False,
    'use_fullscreen_button': False,
    'path_to_docs': 'doc/',
    'repository_branch': 'main',
    'logo': {
        'text': html_title,
    }
}

# -- Options for LaTeX output -------------------------------------------------

latex_engine = 'xelatex'
latex_logo = '../resources/logo.svg'
latex_show_urls = 'inline'

# -- Options for QtHelp output -------------------------------------------------

# These are automatically set when the docs are generated using CMake.
qthelp_basename = build_info['name']
qthelp_namespace = 'org.dankeenan.{}'.format(build_info['name'])
