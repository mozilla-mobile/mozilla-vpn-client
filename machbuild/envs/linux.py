# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from machbuild.envs.common import Bootstrap

import os
import subprocess
import sys

# TODO: support !ubuntu

class LinuxBootstrap(Bootstrap):
    DEBIAN_PACKAGES = [
        'build-essential',
        'libdbus-glib-1-dev',
        'libegl-dev',
        'libfontconfig1-dev',
        'libgl-dev',
        'libglib2.0-dev',
        'libpolkit-gobject-1-dev',
        'libxkbcommon-x11-dev',
        'unzip',
    ]

    CLANG_DIRNAME = 'clang+llvm-13.0.1-x86_64-linux-gnu-ubuntu-18.04'
    CLANG_FILENAME = 'clang+llvm-13.0.1-x86_64-linux-gnu-ubuntu-18.04.tar.xz'
    CLANG_URL = 'https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.1/clang+llvm-13.0.1-x86_64-linux-gnu-ubuntu-18.04.tar.xz'
    CLANG_SHA256 = '84a54c69781ad90615d1b0276a83ff87daaeded99fbc64457c350679df7b4ff0'

    GO_FILENAME = 'go1.18.linux-amd64.tar.gz'
    GO_URL = 'https://dl.google.com/go/go1.18.linux-amd64.tar.gz'
    GO_SHA256 = 'e85278e98f57cdb150fe8409e6e5df5343ecb13cebf03a5d5ff12bd55a80264f'

    def __init__(self, mach):
        super().__init__(mach)

    def install_system_packages(self):
        print('Installing system packages')

        command = ['apt-get', 'install', '-y']
        command.extend(self.DEBIAN_PACKAGES)

        self.run_as_root(command)

        print(f'Downloading ${self.GO_URL}...')
        self.http_download_and_save(self.GO_URL, self.GO_FILENAME, self.GO_SHA256)
        self.extract_from_cache(self.GO_FILENAME, 'go')

    def install_clang(self):
        print(f'Downloading {self.CLANG_URL}...')
        self.http_download_and_save(self.CLANG_URL, self.CLANG_FILENAME, self.CLANG_SHA256)
        self.extract_from_cache(self.CLANG_FILENAME, self.CLANG_DIRNAME, 'clang')
        print(f'{self.CLANG_FILENAME} downloaded.')

    def install_qt(self):
        print('Checking aqt...')
        if self.which('aqt'):
            aqt = 'aqt'
        else:
            localpath = os.path.expanduser("~/.local")
            aqt = os.path.join(localpath, 'bin', 'aqt')
            if not os.path.isfile(aqt) or not os.access(aqt, os.X_OK):
                raise NotImplementedError('Unable to find `aqt`. Have you actually installed it?')

        print('Installing qt...')
        self.remove_dir_or_file(os.path.join(self.mach.state_dir, 'qt'))
        command = [aqt, 'install-qt', '-O',
                   os.path.join(self.mach.state_dir, 'qt'),
                  'linux', 'desktop', '6.2.3',
                  '-m', 'qtcharts', 'qtwebsockets', 'qt5compat', 'qtnetworkauth']
        subprocess.check_call(command, stdin=sys.stdin)

    def ensure_rust_modern(self):
        print('Installing rust... TODO')
        # TODO

    def which(self, name, *extra_search_dirs):
        search_dirs = os.environ['PATH'].split(os.pathsep)
        search_dirs.extend(extra_search_dirs)

        for path in search_dirs:
            test = os.path.join(path, name)
            if os.path.isfile(test) and os.access(test, os.X_OK):
                return test

        return None

    def run_as_root(self, command):
        if os.geteuid() != 0:
            if self.which('sudo'):
                command.insert(0, 'sudo')
            else:
                command = ['su', 'root', '-c', ' '.join(command)]

        print('Executing as root:', subprocess.list2cmdline(command))

        subprocess.check_call(command, stdin=sys.stdin)

def bootstrap(mach):
    return LinuxBootstrap(mach).run()
