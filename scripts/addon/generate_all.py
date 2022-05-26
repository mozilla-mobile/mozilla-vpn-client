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

for file in os.listdir(addons_path):
    if not file.startswith("tutorial_") and not file.startswith("guide_"):
        continue
    addon_path = os.path.join(addons_path, file, "manifest.json")

    build_args = [sys.executable, build_path, addon_path, generated_path]
    if args.qtpath:
        build_args.append("-q")
        build_args.append(args.qtpath)
    subprocess.call(build_args)
