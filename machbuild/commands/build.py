#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import sys
import subprocess

def run(mach, args):
    print('Configure the PATH env...')
    paths = [
        os.path.join(mach.state_dir, 'clang', 'bin'),
        os.path.join(mach.state_dir, 'go', 'bin'),
        os.path.join(mach.state_dir, 'qt', '6.2.3', 'gcc_64', 'bin'),
    ]
    os.environ['PATH'] = ':'.join(paths) + ':' + os.environ['PATH']

    # TODO: Use the script as a module!
    print('Importing languages...')
    command = [os.path.join('scripts', 'utils', 'import_languages.py')]
    subprocess.check_call(command, stdin=sys.stdin)

    print('Running qmake...')
    command = [os.path.join(mach.state_dir, 'qt', '6.2.3', 'gcc_64', 'bin', 'qmake')]
    subprocess.check_call(command, stdin=sys.stdin)

    print('Compiling...')
    command = ['make', '-j8']
    subprocess.check_call(command, stdin=sys.stdin)

    print('Done!')

def initialize(mach):
    mach.register_command('build', 'Build the client.', run)
