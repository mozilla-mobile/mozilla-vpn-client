#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from collections import defaultdict
from glob import glob
import xml.etree.ElementTree as etree
import json
import os
import subprocess
import sys
import tempfile

script_folder = os.path.abspath(os.path.dirname(__file__))
vpn_root_folder = os.path.realpath(os.path.join(script_folder, os.pardir, os.pardir))

def fileContentsAsJSON(filepath):
    try:
        with open(filepath, 'r') as file:
            content = json.load(file)

            if not content:
                print(f"No content found in: {filepath}")
                sys.exit(1)
            return content
    except FileNotFoundError:
        print(f"File not found: {filepath}")
        sys.exit(1)
    except Exception as e:
        print(f"An error occurred while loading {filepath}: {e}")
        sys.exit(1)

# Check if it uses shared strings. If so, pull the value for “title”, “subtitle”, then look within "blocks"
def getSharedStringsInManifest(mainifest_contents):
    shared_string_ids = []
    # If it includes the key “message” and if within "message", there is a “usesSharedStrings” key that is true...
    if manifest_contents.get("message", {}).get("usesSharedStrings", False):
        #...collect all the string IDs
        if "title" in manifest_contents["message"]:
            shared_string_ids.append(manifest_contents["message"]["title"])
        else:
            print(f"No title found for: {addon}")
            sys.exit(1)
        if "subtitle" in manifest_contents["message"]:
            # It is acceptable if there is no subtitle (but title is required)
            shared_string_ids.append(manifest_contents["message"]["subtitle"])
        if not "blocks" in manifest_contents["message"]:
            print(f"No blocks found for: {addon}")
            sys.exit(1)
        for block in manifest_contents["message"]["blocks"]:
            if not "content" in block:
                print(f"No content found in {addon} for {block}")
                sys.exit(1)
            if isinstance(block["content"], str):
                shared_string_ids.append(block["content"])
            elif isinstance(block["content"], list):
                # This is the contents of a bulleted or ordered list; go one layer deeper.
                for list_item in block["content"]:
                    if isinstance(list_item["content"], str):
                        shared_string_ids.append(list_item["content"])
                    else:
                        print(f"No content found in {addon} for {list_item}")
                        sys.exit(1)
            else:
                print(f"Content found in {addon} was unknown type for {block}")
                sys.exit(1)
    return shared_string_ids

### 1. Check Addons' shared strings - all string IDs should be present in the translation file. ###
# Find list of all addon folders
addon_path = os.path.join(vpn_root_folder, "addons")
try:
    addon_list = [item for item in os.listdir(addon_path) if os.path.isdir(os.path.join(addon_path, item))]
except FileNotFoundError:
    print(f"Path not found: {addon_path}")
    sys.exit(1)
except Exception as e:
    print(f"An error occurred when finding all addons: {e}")
    sys.exit(1)

if len(addon_list) == 0:
    print(f"No addons found")
    sys.exit(1)

shared_string_ids = []

# For each, open the manifest file and pull out the shared strings
for addon in addon_list:
    manifest_path = os.path.join(addon_path, addon, "manifest.json")
    manifest_contents = fileContentsAsJSON(manifest_path)
    shared_string_ids = shared_string_ids + getSharedStringsInManifest(manifest_contents)

# Create temporary translation file
tmp_path = tempfile.mkdtemp()
shared_addon_strings = os.path.join(addon_path, "strings.yaml")
shared_addons_tmp_file = os.path.join(tmp_path, "strings.xliff")
generate_addon_python_file = os.path.join(vpn_root_folder, "scripts", "utils", "generate_shared_addon_xliff.py")
p = subprocess.run(['python', generate_addon_python_file, '-i', shared_addon_strings, '-o', shared_addons_tmp_file])

# Check that the shared strings are present in the translation file
if not os.path.isfile(shared_addons_tmp_file):
    print(f"Unable to find {shared_addons_tmp_file}")
    sys.exit(1)
try:
    with open(shared_addons_tmp_file, 'r') as file:
        shared_string_content = file.read()
        if not shared_string_content:
            print(f"No content found in: {shared_addons_tmp_file}")
            sys.exit(1)
except FileNotFoundError:
    print(f"File not found: {shared_addons_tmp_file}")
    sys.exit(1)
except Exception as e:
    print(f"An error occurred while loading {shared_addons_tmp_file}: {e}")
    sys.exit(1)

for shared_string_id in shared_string_ids:
    string_id_unit = "<trans-unit id=\"" + shared_string_id + "\">"
    if string_id_unit not in shared_string_content:
        print(f"{string_id_unit} not found in shared string file")
        sys.exit(1)

### 2. Check all strings for overall issues. ###

# Paths are relative to the root folder
paths = [
    "translations.ts",
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
