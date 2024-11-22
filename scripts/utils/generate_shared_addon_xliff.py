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
from shared import write_en_language, find_qtbinpath


# parseYAMLTranslationStrings prints comments and values as a list. This must be flattened for write_en_language.
def prune_lists_to_strings(data):
    for value in data.values():
        if isinstance(value, dict):
            for sub_key, sub_value in value.items():
                if isinstance(sub_value, list) and sub_value:
                    value[sub_key] = sub_value[0]
        else:
            sys.exit("Unexpected input")
    return data

def use_proper_id(data):
    return_data = {}
    for value in data.values():
        string_id = value["string_id"]
        return_data[string_id] = {
            "value": value["value"],
            "comments": value["comments"]
        }
    return return_data

# Make sure we have all the required things
# Lookup our required tools for addon generation.

qtbinpath = find_qtbinpath(None)
lconvert = os.path.join(qtbinpath, 'lconvert')
if lconvert is None:
    sys.exit("Unable to locate lconvert path.", file=sys.stderr)

parser = argparse.ArgumentParser(description="Prepare shared addon strings for translation repo")
parser.add_argument(
    "-i",
    "--infile",
    default=None,
    dest="infile",
    required=True,
    help="File for input, including .yaml extension"
)
parser.add_argument(
    "-o",
    "--outfile",
    default=None,
    dest="outfile",
    required=True,
    help="File for output, including .xliff extension"
)
args = parser.parse_args()

print("Preparing the addons shared string file")
print("First, pull in the strings from the YAML file")
translation_strings = parseYAMLTranslationStrings(args.infile)
translation_strings = prune_lists_to_strings(translation_strings)
# parseYAMLTranslationStrings gives a different ID than we want to use
translation_strings = use_proper_id(translation_strings)

print("Then, write the strings to a .ts file")
tmp_path = tempfile.mkdtemp()
ts_file = os.path.join(tmp_path, "sharedAddonsStrings.ts")
write_en_language(ts_file, translation_strings)

print("Finally, convert the ts file into an xliff file")
os.system(f"{lconvert} -if ts -i {ts_file} -of xlf -o {args.outfile}")
