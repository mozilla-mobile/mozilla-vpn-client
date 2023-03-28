#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from collections import defaultdict
from glob import glob
import xml.etree.ElementTree as etree
import json
import os
import sys

script_folder = os.path.abspath(os.path.dirname(__file__))
vpn_root_folder = os.path.realpath(os.path.join(script_folder, os.pardir, os.pardir))

# Paths are relative to the root folder
paths = [
    "translations_*.ts",
    "addon_ts/*.ts",
]

# Get a list of files to analyze
files = []
for p in paths:
    for f in glob(f"{vpn_root_folder}/{p}", recursive=False):
        files.append(f)
files.sort()

# Load the exceptions file. String IDs can be added to each category to
# ignore errors.
with open(os.path.join(script_folder, "exceptions.json")) as f:
    exceptions = json.load(f)

misused_characters = {
    "'": "Invalid character ' in string. Use a typographic apostrophe ’ instead, or add an exception.",
    '"': 'Invalid character " in string. Use typographic quotes “” instead, or add an exception.',
    "...": 'Invalid "..." in string: use proper ellipsis (…) instead.',
}
errors = defaultdict(dict)

for f in files:
    relative_filename = os.path.relpath(f, vpn_root_folder)
    # Load the .ts file
    ts_file = os.path.join(f)
    tree = etree.parse(ts_file)
    root = tree.getroot()

    for message in root.findall(".//message"):
        message_id = message.get("id")
        source_text = message.find("./source").text
        source_comment = (
            message.find("./extracomment").text
            if message.find("./extracomment") is not None
            else ""
        )
        print(f, message_id)

        if source_text is None:
            # Check if the string is empty
            if message_id not in exceptions["empty"]:
                errors[relative_filename][message_id] = "String should not be empty."
        else:
            # Check if there are misused characters in the strings
            if message_id not in exceptions["characters"]:
                for character in misused_characters.keys():
                    if character in source_text:
                        errors[relative_filename][message_id] = misused_characters[
                            character
                        ]

            # Check if there are variables in the string but no comments
            if message_id not in exceptions["comments"]:
                if "%" in source_text and source_comment is None:
                    errors[relative_filename][
                        message_id
                    ] = "Strings with variables should always have a comment."

if errors:
    print("\n----\nERRORS:")
    for filename, errors in errors.items():
        for message_id, error in errors.items():
            print(f"{message_id} ({filename}):\n  {error}")
    sys.exit(1)
else:
    print("\nNo errors found.")
