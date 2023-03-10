#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import hashlib
import json
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
parser.add_argument(
    "-p",
    "--addon_path",
    default=None,
    dest="addonpath",
    help="The path of the addons. If not set, we use `./addons/`.",
)
args = parser.parse_args()

lang_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "utils", "import_languages.py")
build_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "build.py")
index_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "index.py")
addons_path = os.path.join(
    os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))),
    "addons",
)

if args.addonpath:
    addons_path = args.addonpath

lang_cmd = [sys.executable, lang_path]
if args.qtpath:
    lang_cmd.append("-q")
    lang_cmd.append(args.qtpath)
subprocess.call(lang_cmd)

generated_path = os.path.join(addons_path, "generated")
if not os.path.isdir(generated_path):
    os.mkdir(generated_path)
generated_path = os.path.join(generated_path, "addons")
if not os.path.isdir(generated_path):
    os.mkdir(generated_path)

generated_addons = []
for file in os.listdir(addons_path):
    addon_path = os.path.join(addons_path, file, "manifest.json")
    if not os.path.exists(addon_path):
       print(f"Ignoring path {file}.")
       continue

    build_cmd = [sys.executable, build_path, "vpn", addon_path, generated_path]
    if args.qtpath:
        build_cmd.append("-q")
        build_cmd.append(args.qtpath)
    subprocess.call(build_cmd)

    generated_addons.append(os.path.join(generated_path, file + ".rcc"))

## Generate the index.
index_cmd = [sys.executable, index_path, '-o', os.path.join(generated_path, 'manifest.json')]
subprocess.call(index_cmd + generated_addons)
