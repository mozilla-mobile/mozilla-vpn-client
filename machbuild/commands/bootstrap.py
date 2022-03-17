#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import importlib.util
import os
import sys
import machbuild.env

def ask_env(mach):
    supported_envs = []
    for env in machbuild.env.envs:
        if sys.platform.startswith(env['platform']):
            supported_envs.append(env)

    if len(supported_envs) == 0:
        raise NotImplementedError('Unsupported platform (yet?')

    while True:
        print('Please choose the version of Firefox you want to build:')
        for i in range(len(supported_envs)):
            print(f'  {i+1}. {supported_envs[i]["name"]}')
        choice = input('Your choice: ')
        try:
            choice = int(choice)
            if 0 < choice <= len(supported_envs):
                machbuild.env.set_current_env(mach, supported_envs[choice-1]["name"])
                return machbuild.env.current_env(mach)["env"]
        except ValueError:
            pass
        print("ERROR! Please enter a valid option!")

def run(mach, args):
    env_file = ask_env(mach)
    spec = importlib.util.spec_from_file_location('env', os.path.join(mach.topdir_path, 'machbuild', env_file))
    env = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(env)
    return env.bootstrap(mach)

def initialize(mach):
    mach.register_command('bootstrap', 'Install required system packages for building.', run)
