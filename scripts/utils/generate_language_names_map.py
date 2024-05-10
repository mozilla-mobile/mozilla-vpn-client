#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import argparse
import xml.etree.ElementTree as etree

def extract_strings_with_id(directory):
    language_strings = {}

    for root, dirs, files in os.walk(directory):
        if "extras.xliff" in files:
            extras_xliff_path = os.path.join(root, "extras.xliff")
            lang_code = os.path.basename(root)
            if lang_code not in language_strings:
                language_strings[lang_code] = {}

            tree = etree.parse(extras_xliff_path)
            root = tree.getroot()

            for trans_unit in root.findall('.//{urn:oasis:names:tc:xliff:document:1.2}trans-unit'):
                unit_id = trans_unit.get('id')
                if unit_id.startswith('languages.'):
                    if lang_code == 'en':
                        value = trans_unit.find('.//{urn:oasis:names:tc:xliff:document:1.2}source')
                    else:
                        value = trans_unit.find('.//{urn:oasis:names:tc:xliff:document:1.2}target')

                    if value is not None and value.text is not None:
                        source_text = value.text.strip()
                        language_strings[lang_code][unit_id.replace('languages.', '')] = source_text

    return language_strings

def generate_cpp_header(language_strings, output_file):
    with open(output_file, 'w', encoding="utf-8") as f:
        f.write('#ifndef LANGUAGE_STRINGS_H\n')
        f.write('#define LANGUAGE_STRINGS_H\n\n')
        f.write('#include <QMap>\n#include <QString>\n\n')
        f.write('namespace LanguageStrings {\n')
        f.write('const QMap<QString, QString> NATIVE_LANGUAGE_NAMES = {\n')
        for lang_code, strings in language_strings.items():
            f.write('    {"' + lang_code + '", "' + strings.get(lang_code, strings.get("en", "")) + '"},\n')
        f.write('};\n}\n\n#endif\n')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Extract language name strings from "extras.xliff" files.')
    parser.add_argument('directory', type=str, help='Path to the i18n directory')
    parser.add_argument('output_file', type=str, help='Output file name')
    args = parser.parse_args()

    directory_path = args.directory
    output_file = args.output_file

    language_strings = extract_strings_with_id(directory_path)
    generate_cpp_header(language_strings, output_file)
