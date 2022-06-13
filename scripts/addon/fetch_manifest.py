#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import requests
import os

# Parse arguments to locate the input and output files.
parser = argparse.ArgumentParser(
    description="Fetch the final addon manifest and generate the manifest.qrc"
)
parser.add_argument(
    "-o",
    "--output",
    metavar="DIR",
    type=str,
    action="store",
    help="Output directory for generated files",
)

args = parser.parse_args()

if args.output is None:
    args.output = os.path.join(
        os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))),
        "addons",
    )
    args.output = os.path.join(args.output, "generated")
    if not os.path.isdir(args.output):
        os.mkdir(args.output)

args.output = os.path.join(args.output, "addons")
if not os.path.isdir(args.output):
    os.mkdir(args.output)

# TODO: write here the final URL for the manifest for 2.9
manifestJson = requests.get('https://mozilla-mobile.github.io/mozilla-vpn-client/addons/manifest.json')
with open(os.path.join(args.output, "manifest.json"), "wb") as f:
  f.write(manifestJson.content)

with open(os.path.join(args.output, "manifest.qrc"), "w") as f:
  f.write('''
<RCC>
  <qresource prefix="/addons">
     <file>manifest.json</file>
  </qresource>
</RCC>
''')
