#!/usr/bin/python3

import sys
import xml.etree.ElementTree as ET


def main():
    if len(sys.argv) != 3:
        print('Usage: {} <sphinx qhcp> <out qhcp>'.format(sys.argv[0]), file=sys.stderr)
        exit(1)

    sphinx_qhcp = ET.parse(sys.argv[1])
    assistant = sphinx_qhcp.getroot().find('assistant')
    if assistant is None:
        raise RuntimeError('assistant key not found.')

    # Application icon.
    application_icon = get_config_element(assistant, 'applicationIcon')
    application_icon.text = 'logo.svg'

    # Disable filters.
    enable_filter_functionality = get_config_element(assistant, 'enableFilterFunctionality')
    enable_filter_functionality.text = 'false'
    enable_filter_functionality.set('visible', 'false')

    # Disable the documentation manager (i.e. show only our docs).
    enable_documentation_manager = get_config_element(assistant, 'enableDocumentationManager')
    enable_documentation_manager.text = 'false'

    # Hide address bar.
    enable_address_bar = get_config_element(assistant, 'enableAddressBar')
    enable_address_bar.text = 'false'
    enable_address_bar.set('visible', 'false')

    sphinx_qhcp.write(sys.argv[2], encoding='utf-8', xml_declaration=True)


def get_config_element(assistant: ET.Element, key: str) -> ET.Element:
    element = assistant.find(key)
    if element is None:
        element = ET.SubElement(assistant, key)
    return element


if __name__ == '__main__':
    main()
