#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import os
import subprocess

parser = argparse.ArgumentParser(description="Generate an addon package")
parser.add_argument(
    "-q",
    "--qt_path",
    default=None,
    dest="qtpath",
    help="The QT binary path. If not set, we try to guess.",
)
args = parser.parse_args()

# Use CMake to build the addons.
# TODO: At some point we should stop generating in the source directory.
test_addons_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "tests", "functional", "addons")
subprocess.call(['cmake', '-S', test_addons_path, '-B', os.path.join(test_addons_path, 'generated')])
subprocess.call(['cmake', '--build', os.path.join(test_addons_path, 'generated')])
