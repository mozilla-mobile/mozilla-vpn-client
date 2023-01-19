#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import os
import subprocess
import sys

parser = argparse.ArgumentParser(description="Generate an addon package")
parser.add_argument(
    "-q",
    "--qt_path",
    default=None,
    dest="qtpath",
    help="The QT binary path. If not set, we try to guess.",
)
args = parser.parse_args()

generateall_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "generate_all.py")

# Generate production addons files
build_cmd = [sys.executable, generateall_path]
if args.qtpath:
    build_cmd.append("-q")
    build_cmd.append(args.qtpath)
subprocess.call(build_cmd)

# Generate test addons files
test_addons_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "tests", "functional", "addons")
for file in os.listdir(test_addons_path):
    manifest_path = os.path.join(test_addons_path, file, "manifest.json")
    if os.path.exists(manifest_path):
       print(f"Ignoring path {file} because the manifest already exists.")
       continue

    build_cmd = [sys.executable, generateall_path, "-p", os.path.join(test_addons_path, file)]
    if args.qtpath:
        build_cmd.append("-q")
        build_cmd.append(args.qtpath)
    subprocess.call(build_cmd)
