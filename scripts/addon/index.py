#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import hashlib
import json
import os

parser = argparse.ArgumentParser(description='Generate an addon index manifest')
parser.add_argument(
    'input',
    nargs='*',
    metavar='ADDON',
    help='Addon files to include in the manifest'
)
parser.add_argument(
    '-o',
    '--output',
    default='manifest.json',
    dest='output',
    metavar='FILE',
    help='Write the index manifest to FILE (default: manifest.json)',
)
args = parser.parse_args()

## If no addons were specified, get them from the current directory listing.
if len(args.input) == 0:
    for filename in os.listdir():
        if os.path.splitext(filename)[1] == '.rcc':
            args.input.append(filename)

## Compute the ID and hashes for each addon.
addons = []
for filename in args.input:
    addon_id = os.path.splitext(os.path.basename(filename))[0]

    with open(filename, 'rb') as f:
        sha256 = hashlib.sha256(f.read()).hexdigest()
        addons.append({ 'id': addon_id, 'sha256': sha256 })

## Write the addon index manifest.
index = {
    'api_version': '0.1',
    'addons': addons,
}

with open(args.output, 'w') as f:
    f.write(json.dumps(index, indent=2))