#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import importlib.util
import os
import sys

envs = [
    {
        'name': 'linux builds',
        'platform': 'linux',
        'env': 'envs/linux.py',
    },
#    {
#        'name': 'android builds on linux',
#        'platform': 'linux',
#        'env': 'envs/linux_android.py',
#    },
#    {
#        'name': 'macos builds',
#        'platform': 'darwin',
#        'env': 'envs/macos.py',
#    },
#    {
#        'name': 'ios builds on macos',
#        'platform': 'darwin',
#        'env': 'envs/ios.py',
#    },
#    {
#        'name': 'windows builds',
#        'platform': 'win',
#        'env': 'envs/windows.py',
#    },
#    {
#        'name': 'wasm builds on linux',
#        'platform': 'linux',
#        'env': 'envs/wasm_linux.py',
#    },
]

def ask_env():
    supported_envs = []
    for env in envs:
        if sys.platform.startswith(env['platform']):
            supported_envs.append(env)

    if len(supported_envs) == 0:
        print('Unsupported platform (yet?')
        return 1

    while True:
        print('Please choose the version of Firefox you want to build:')
        for i in range(len(supported_envs)):
            print(f'  {i+1}. {supported_envs[i]["name"]}')
        choice = input('Your choice: ')
        try:
            choice = int(choice)
            if 0 < choice <= len(supported_envs):
                return supported_envs[choice-1]["env"]
        except ValueError:
            pass
        print("ERROR! Please enter a valid option!")

def run(mach, args):
    env_file = ask_env()
    spec = importlib.util.spec_from_file_location('env', os.path.join(mach.topdir_path, 'machbuild', env_file))
    env = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(env)
    return env.bootstrap(mach)

def initialize(mach):
    mach.register_command('bootstrap', 'Install required system packages for building.', run)
