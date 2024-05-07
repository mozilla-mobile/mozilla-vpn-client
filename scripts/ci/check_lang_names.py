#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import sys
import xml.etree.ElementTree as etree


if __name__ == "__main__":
    script_path = os.path.dirname(os.path.abspath(__file__))

    ###
    # 1. Grab the list of expected languages in the i18n submodule

    i18n_path = os.path.join(script_path, '..', '..', '3rdparty', 'i18n')
    if not os.path.exists(i18n_path):
        print(f"i18n submodule not found in path {i18n_path}")
        sys.exit(1)

    # The i18n folder has one folder per language
    # and the name of the folder is the code of the language.
    #
    # Therefore the list of expected languages
    # is the list of subfolder in that directory.
    expected_languages = []
    for entry in os.listdir(i18n_path):
        full_path = os.path.join(i18n_path, entry)
        if os.path.isdir(full_path) and not entry.startswith("."):
            expected_languages.append(entry)

    if not expected_languages:
        print("It seems the i18n submodule directory is empty")
        sys.exit(1)

    ###
    # 2. Grab the list of language name strings in extras.xliff

    xliff_path = os.path.join(script_path, '..', '..', 'src', 'translations', 'extras', 'extras.xliff')
    if not os.path.exists(i18n_path):
        print(f"extras.xliff not found in path {xliff_path}")
        sys.exit(1)

    tree = etree.parse(xliff_path)
    root = tree.getroot()

    found_languages = []
    for trans_unit in root.findall('.//{urn:oasis:names:tc:xliff:document:1.2}trans-unit'):
        unit_id = trans_unit.get('id')
        if unit_id.startswith('languages.'):
            found_languages.append(unit_id.split('.')[1])

    ###
    # 3. Check if there are languages in the i18n folder not in the extras.xliff

    diff = [lang_code for lang_code in expected_languages if lang_code not in found_languages]
    if diff:
        print(f"Languages {diff} found in the i18n submodule do not have language names strings in extras.xliff!")
        sys.exit(1)

    print("Language names are up to date")
