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
args = parser.parse_args()

build_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "build.py")
addons_path = os.path.join(
    os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))),
    "addons",
)

generated_path = os.path.join(addons_path, "generated")
if not os.path.isdir(generated_path):
    os.mkdir(generated_path)
generated_path = os.path.join(generated_path, "addons")
if not os.path.isdir(generated_path):
    os.mkdir(generated_path)

addons = []
for file in os.listdir(addons_path):
    addon_path = os.path.join(addons_path, file, "manifest.json")
    if not os.path.exists(addon_path):
       print(f"Ignoring path {file}.")
       continue

    build_cmd = [sys.executable, build_path, addon_path, generated_path]
    if args.qtpath:
        build_cmd.append("-q")
        build_cmd.append(args.qtpath)
    subprocess.call(build_cmd)

    generated_addon_path = os.path.join(generated_path, file + ".rcc")
    if not os.path.exists(generated_addon_path):
        exit(f"Expected addon file {generated_addon_path}")

    with open(generated_addon_path,"rb") as f:
        sha256 = hashlib.sha256(f.read()).hexdigest();
        addons.append({ 'id': file, 'sha256': sha256 })

index = {
  'api_version': '0.1',
  'addons': addons,
}

with open(os.path.join(generated_path, "manifest.json"), "w") as f:
  f.write(json.dumps(index, indent=2))
