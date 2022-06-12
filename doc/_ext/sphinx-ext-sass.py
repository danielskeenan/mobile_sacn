"""
Sass extension.

Define a list of sass files called ``sass_files`` in the config file.

"""
from pathlib import Path

import sass
import sphinx.application
from sphinx.errors import SphinxError, ConfigError
from sphinx.util import logging

VERSION = '0.1'
logger = logging.getLogger(__name__)


class BadSassFileError(ConfigError):
    """
    Raised when the sass file cannot be opened.
    """

    def __init__(self, path: Path):
        ConfigError.__init__(self, path)


class SassError(SphinxError):
    """
    Raised to wrap a Sass compile error for Sphinx.
    """

    def __init__(self, path: Path, e: sass.CompileError):
        SphinxError.__init__(self, 'Error compiling {}: {}'.format(path, e))


def _compile_sass(app: sphinx.application.Sphinx, config):
    out_static_path = Path(app.outdir) / '_static'
    out_static_path.mkdir(parents=True, exist_ok=True)
    for sass_file_path in [Path(p) for p in config.sass_files]:
        # Create an absolute path.
        if sass_file_path.is_absolute():
            abs_sass_file_path = sass_file_path
        else:
            abs_sass_file_path = (Path(app.confdir) / sass_file_path).absolute()

        # Ensure we can read the file.
        if not abs_sass_file_path.is_file():
            raise BadSassFileError(abs_sass_file_path)

        # Compile Sass
        logger.info('Compiling {}'.format(abs_sass_file_path.relative_to(app.srcdir)))
        try:
            css = sass.compile(filename=str(abs_sass_file_path))
        except sass.CompileError as e:
            raise SassError(sass_file_path, e)

        # Write to temp file.
        out_file_path = (out_static_path / abs_sass_file_path.stem).with_suffix('.css')
        with out_file_path.open('wt', encoding='utf-8') as out_file:
            out_file.write(css)
        app.add_css_file(str(out_file_path.relative_to(out_static_path)))


def setup(app: sphinx.application.Sphinx):
    # Config
    app.add_config_value('sass_files', default=[], rebuild='html')

    app.connect('config-inited', _compile_sass)

    return {
        'version': VERSION,
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
