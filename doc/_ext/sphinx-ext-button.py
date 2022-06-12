from typing import Sequence

import sphinx.application
from docutils import nodes
from docutils.parsers.rst import Directive, directives
from sphinx.errors import SphinxError

VERSION = '0.1'


def _color_choice(arg: str):
    return directives.choice(arg, ('primary', 'secondary', 'success', 'danger', 'warning', 'info', 'light', 'dark'))


class Button(Directive):
    option_spec = {
        'color': _color_choice,
        'icon': directives.path,
    }
    has_content = True

    def run(self) -> Sequence[nodes.Node]:
        # Raise an error if the directive does not have contents.
        if not self.content:
            raise SphinxError("button must have a body.")

        text = '\n'.join(self.content)
        button_node = nodes.inline(rawsource=text)
        self.state.nested_parse(self.content, self.content_offset, button_node)

        # The nested parse will create a paragraph tag for textual contents, which won't work.
        # Change paragraph tags to inlines.
        for child in button_node.children:
            if isinstance(child, nodes.paragraph):
                inline = nodes.inline(text=child.astext())
                child.replace_self(inline)

        # Apply styles.
        button_node['classes'].extend(('btn', 'color-{}'.format(self.options['color'])))

        return [button_node]


def setup(app: sphinx.application.Sphinx):
    app.add_directive('button', Button)

    return {
        'version': VERSION,
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
