#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from lxml import etree
import json
import os
import sys

script_folder = os.path.abspath(os.path.dirname(__file__))
vpn_root_folder = os.path.join(script_folder, os.pardir, os.pardir)

# Extract all strings in the .ts file
srcFile = os.path.join(vpn_root_folder, "src", "src.pro")
os.system(f"lupdate {srcFile} -ts")

# Load the exceptions file. String IDs can be added to each category to
# ignore errors.
with open(os.path.join(script_folder, "exceptions.json")) as f:
    exceptions = json.load(f)

# Load the .ts file
ts_file = os.path.join(vpn_root_folder, "translations", "en", "mozillavpn_en.ts")
tree = etree.parse(ts_file)
root = tree.getroot()

misused_characters = {
    "'": "Invalid character ' in string. Use a typographic apostrophe ’ instead, or add an exception.",
    '"': 'Invalid character " in string. Use typographic quotes “” instead, or add an exception.',
    "...": 'Invalid "..." in string: use proper ellipsis (…) instead.',
}
errors = {}
for message in root.xpath("//message"):
    message_id = message.get("id")
    source_text = message.xpath("./source")[0].text
    source_comment = (
        message.xpath("./extracomment")[0].text
        if message.xpath("./extracomment")
        else ""
    )

    # Check if there are misused characters in the strings
    if message_id not in exceptions["characters"]:
        for character in misused_characters.keys():
            if character in source_text:
                errors[message_id] = misused_characters[character]

    # Check if the string is empty
    if message_id not in exceptions["empty"]:
        if source_text == "":
            errors[message_id] = "String should not be empty."

    # Check if there are variables in the string but no comments
    if message_id not in exceptions["comments"]:
        if "%" in source_text and source_comment == "":
            errors[message_id] = "Strings with variables should always have a comment."

if errors:
    print("\n----\nERRORS:")
    for message_id, error in errors.items():
        print(f"{message_id}:\n  {error}")
    sys.exit(1)
else:
    print("\nNo errors found.")
