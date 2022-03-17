# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import importlib.util
import os
import sys

class Mach:
    def __init__(self, topdir_path):
        self.topdir_path = topdir_path
        self._commands = []

        self.state_dir = os.path.expanduser("~/.mozvpnbuild")
        if not os.path.exists(self.state_dir):
            print("Creating default state directory: {}".format(self.state_dir))
        os.makedirs(self.state_dir, mode=0o770, exist_ok=True)

        self.cache_dir = os.path.join(self.state_dir, "cache")
        if not os.path.exists(self.cache_dir):
            print("Creating default cache directory: {}".format(self.cache_dir))
        os.makedirs(self.cache_dir, mode=0o770, exist_ok=True)

        commands_path = os.path.join(topdir_path, 'machbuild', 'commands')
        for command_file in os.listdir(commands_path):
            if not command_file.endswith('.py'):
                continue

            spec = importlib.util.spec_from_file_location('command', os.path.join(commands_path, command_file))
            command = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(command)
            command.initialize(self)

    def register_command(self, name, desc, callback):
        self._commands.append({
           'name': name,
           'desc': desc,
           'callback': callback
        })

    def run(self, args):
        if len(args) > 0:
            for command in self._commands:
                if command['name'] == args[0]:
                    return command['callback'](self, args)
            print(f'error: command not found - {args[0]}')
            return 1

        print('usage: mach command [command arguments]\n\nCommands:\n')
        for command in self._commands:
            length = len(command['name'])
            print('  %s' % command["name"], end ="")
            for i in range(length, 20): print(' ', end ="")
            print(command["desc"])
        print('')
        return 0
