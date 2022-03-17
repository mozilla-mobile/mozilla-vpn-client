#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os

envs = [
    {
        'name': 'linux builds',
        'platform': 'linux',
        'env': 'envs/linux.py',
        'qt_version': '6.2.3',
        'qt_platform': 'gcc_64',
        'extra_import_params': None,
    },
#    {
#        'name': 'android builds on linux',
#        'platform': 'linux',
#        'env': 'envs/linux_android.py',
#        'qt_version': '6.2.3',
#        'qt_platform': 'gcc_64',
#        'extra_import_params': None,
#    },
    {
        'name': 'macos builds',
        'platform': 'darwin',
        'env': 'envs/macos.py',
        'qt_version': '6.2.3',
        'qt_platform': 'macos',
        'extra_import_params': [ '-m' ],
    },
#    {
#        'name': 'ios builds on macos',
#        'platform': 'darwin',
#        'env': 'envs/ios.py',
#        'qt_version': '6.2.3',
#        'qt_platform': 'gcc_64',
#        'extra_import_params': None,
#    },
#    {
#        'name': 'windows builds',
#        'platform': 'win',
#        'env': 'envs/windows.py',
#        'qt_version': '6.2.3',
#        'qt_platform': 'gcc_64',
#        'extra_import_params': None,
#    },
#    {
#        'name': 'wasm builds on linux',
#        'platform': 'linux',
#        'env': 'envs/wasm_linux.py',
#        'qt_version': '6.2.3',
#        'qt_platform': 'gcc_64',
#        'extra_import_params': None,
#    },
]

def set_current_env(mach, env_name):
    file_env = os.path.join(mach.state_dir, "env")
    with open(file_env, 'wb') as f:
        f.write(bytes(env_name, 'utf-8'))

def current_env(mach):
    file_env = os.path.join(mach.state_dir, "env")
    with open(file_env, 'r') as f:
        env_name = str(f.read(4096))
        for env in envs:
            if env["name"] == env_name:
                return env
    return None
