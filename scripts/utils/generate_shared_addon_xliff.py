#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script is used when importing strings upstream into the i18n repo to convert the addons/strings.yaml file into an xliff.

import argparse
import os
import sys
import tempfile

from generate_strings import parseYAMLTranslationStrings

# hack to be able to access shared.py
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../')))
from shared import write_en_language, qtquery


# parseYAMLTranslationStrings prints comments and values as a list. This must be flattened for write_en_language.
def prune_lists_to_strings(data):
    for value in data.values():
        if isinstance(value, dict):
            for sub_key, sub_value in value.items():
                if isinstance(sub_value, list) and sub_value:
                    value[sub_key] = sub_value[0]
        else:
            print("Unexpected input")
            sys.exit(1)
    return data

print("Preparing the addons shared string file")

# Make sure we have all the required things
# Lookup our required tools for addon generation.

qtbinpath = qtquery('qmake', 'QT_INSTALL_BINS')
if qtbinpath is None:
    qtbinpath = qtquery('qmake6', 'QT_INSTALL_BINS')
if qtbinpath is None:
    print('Unable to locate qmake tool.')
    sys.exit(1)
if not os.path.isdir(qtbinpath):
    print(f"QT path is not a diretory: {qtbinpath}")
    sys.exit(1)
lconvert = os.path.join(qtbinpath, 'lconvert')
if lconvert is None:
    print("Unable to locate lconvert path.", file=sys.stderr)
    sys.exit(1)

parser = argparse.ArgumentParser(description="Prepare shared addon strings for translation repo")
parser.add_argument(
    "-o",
    "--outfile",
    default=None,
    dest="outfile",
    help="File for output, including .xliff extension"
)
args = parser.parse_args()
if args.outfile is None:
    print("No output file specified")
    sys.exit(1)

print("First, pull in the strings from the YAML file")
addons_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../addons")
strings_file = os.path.normpath(os.path.join(addons_dir, "strings.yaml"))
translation_strings = parseYAMLTranslationStrings(strings_file)
translation_strings = prune_lists_to_strings(translation_strings)

print("Then, write the strings to a .ts file")
tmp_path = tempfile.mkdtemp()
ts_file = os.path.join(tmp_path, "sharedAddonsStrings.ts")
write_en_language(ts_file, translation_strings)

print("Finally, convert the ts file into an xliff file")
os.system(f"{lconvert} -if ts -i {ts_file} -of xlf -o {args.outfile}")
