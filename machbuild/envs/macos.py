# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from machbuild.envs.common import Bootstrap

import os
import subprocess
import sys

class MacOSBootstrap(Bootstrap):
    CLANG_DIRNAME = 'clang+llvm-13.0.1-x86_64-apple-darwin'
    CLANG_FILENAME = 'clang+llvm-13.0.1-x86_64-apple-darwin.tar.xz'
    CLANG_URL = 'https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.1/clang+llvm-13.0.1-x86_64-apple-darwin.tar.xz'
    CLANG_SHA256 = 'dec02d17698514d0fc7ace8869c38937851c542b02adf102c4e898f027145a4d'

    GO_FILENAME = 'go1.18.darwin-amd64.tar.gz'
    GO_URL = 'https://go.dev/dl/go1.18.darwin-amd64.tar.gz'
    GO_SHA256 = '70bb4a066997535e346c8bfa3e0dfe250d61100b17ccc5676274642447834969'

    def __init__(self, mach):
        super().__init__(mach)

    def install_system_packages(self):
        print('Installing system packages')

        # TODO: check xcode version

        print(f'Downloading {self.GO_URL}...')
        self.http_download_and_save(self.GO_URL, self.GO_FILENAME, self.GO_SHA256)
        self.extract_from_cache(self.GO_FILENAME, 'go')

    def install_clang(self):
        print(f'Downloading {self.CLANG_URL}...')
        self.http_download_and_save(self.CLANG_URL, self.CLANG_FILENAME, self.CLANG_SHA256)
        self.extract_from_cache(self.CLANG_FILENAME, self.CLANG_DIRNAME, 'clang')
        print(f'{self.CLANG_FILENAME} downloaded.')

    def install_qt(self):
        self.install_qt_for_platform('mac')

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
    return MacOSBootstrap(mach).run()
