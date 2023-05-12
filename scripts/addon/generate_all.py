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
parser.add_argument(
    "-p",
    "--addon_path",
    default=None,
    dest="addonpath",
    help="The path of the addons. If not set, we use `./addons/`.",
)
args = parser.parse_args()

if args.addonpath is None:
    args.addonpath = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "addons")

cmake_setup_args = ['cmake', '-S', args.addonpath, '-B', os.path.join(args.addonpath, 'generated')]

if args.qtpath is not None:
    qt_install_libs = subprocess.check_output([os.path.join(args.qtpath, 'qmake'), '-query', 'QT_INSTALL_LIBS'])
    qt_cmake_prefix = os.path.join(qt_install_libs.decode().strip(), 'cmake')
    cmake_setup_args.append(f'-DCMAKE_PREFIX_PATH={qt_cmake_prefix}')

# Use CMake to build the test addons.
# TODO: At some point we should stop generating in the source directory.
subprocess.run(cmake_setup_args, check=True)
subprocess.run(['cmake', '--build', os.path.join(args.addonpath, 'generated')], check=True)
subprocess.run(['cmake', '--install', os.path.join(args.addonpath), '--component', 'addons'])
