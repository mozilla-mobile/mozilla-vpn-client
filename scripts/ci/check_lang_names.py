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

    i18n_path = os.path.join(
        script_path, os.path.pardir, os.path.pardir, "3rdparty", "i18n"
    )
    if not os.path.exists(i18n_path):
        sys.exit(f"i18n submodule not found in path {i18n_path}")

    # The i18n folder has one folder per language
    # and the name of the folder is the code of the language.
    #
    # Therefore the list of expected languages
    # is the list of subfolder in that directory.
    expected_languages = [
        f for f in os.listdir(i18n_path)
        if os.path.isdir(os.path.join(i18n_path, f)) and not f.startswith(".")
    ]

    if not expected_languages:
        sys.exit("It seems the i18n submodule directory is empty")

    ###
    # 2. Grab the list of language name strings in extras.xliff

    xliff_path = os.path.join(
        script_path, os.path.pardir, os.path.pardir, 'src', 'translations',
        'extras', 'extras.xliff'
    )
    if not os.path.exists(xliff_path):
        sys.exit(f"extras.xliff not found in path {xliff_path}")

    tree = etree.parse(xliff_path)
    root = tree.getroot()

    found_languages = []
    for trans_unit in root.findall('.//{urn:oasis:names:tc:xliff:document:1.2}trans-unit'):
        unit_id = trans_unit.get('id')
        if unit_id.startswith('languages.'):
            found_languages.append(unit_id.split('.')[1])

    ###
    # 3. Check if there are languages in the i18n folder not in the extras.xliff

    diff = list(set(expected_languages) - set(found_languages))
    if diff:
        sys.exit(f"Languages {diff} found in the i18n submodule do not have language names strings in extras.xliff!")

    print("Language names are up to date")
