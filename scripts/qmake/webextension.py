#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import shutil
import subprocess
import os

def build():
    os.environ['CARGO_HOME'] = os.path.join(os.getcwd(), '.cargo_home')
    if subprocess.call(['cargo', 'build', '--release']):
        os.exit(1)

def clean():
    if subprocess.call(['cargo', 'clean']):
        os.exit(1)
    if os.path.isdir('vendor'):
        shutil.rmtree('vendor')

parser = argparse.ArgumentParser(description='Compile the browser bridge for the web-extensions')
parser.add_argument('operation', metavar='operation', type=str, nargs=1,
                    help='the operation to execute (build, clean)')

args = parser.parse_args()
args = args.operation[0]

workdir = os.path.join(os.path.dirname(__file__), '..', '..', 'extension', 'bridge')
if not os.path.isdir(workdir):
    print('Unable to find the extension bridge directory: %s' % workdir)
    os.exit(1)

os.chdir(workdir)

if args == 'build':
  build()

elif args == 'clean':
  clean()

else:
  parser.print_help()
