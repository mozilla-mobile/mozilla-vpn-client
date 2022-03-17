#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import sys
import subprocess
import machbuild.env

def run(mach, args):
    env = machbuild.env.current_env(mach)
    if env == None:
        print('No bootstrap yet?')
        return 1

    print('Configure the PATH env...')
    paths = [
        os.path.join(mach.state_dir, 'clang', 'bin'),
        os.path.join(mach.state_dir, 'go', 'bin'),
        os.path.join(mach.state_dir, 'qt', env['qt_version'], env['qt_platform'], 'bin'),
    ]
    print(paths)
    os.environ['PATH'] = ':'.join(paths) + ':' + os.environ['PATH']

    # TODO: Use the script as a module!
    print('Importing languages...')
    command = [os.path.join('scripts', 'utils', 'import_languages.py')]
    if env['extra_import_params'] != None:
        command.extend(env['extra_import_params'])
    subprocess.check_call(command, stdin=sys.stdin)

    print('Running qmake...')
    command = [os.path.join(mach.state_dir, 'qt', env['qt_version'], env['qt_platform'], 'bin', 'qmake')]
    subprocess.check_call(command, stdin=sys.stdin)

    print('Compiling...')
    command = ['make', '-j8']
    subprocess.check_call(command, stdin=sys.stdin)

    print('Done!')

def initialize(mach):
    mach.register_command('build', 'Build the client.', run)
