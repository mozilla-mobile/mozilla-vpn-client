# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import hashlib
import os
import shutil
import subprocess
from urllib.request import urlopen

class Bootstrap:
    def __init__(self, mach):
        self.mach = mach

    def run(self):
        self.install_system_packages()
        self.install_qt()
        self.install_clang()
        self.ensure_rust_modern()
        return 0

    def install_system_packages(self):
        raise NotImplementedError(
            'Cannot bootstrap MozillaVPN: '
            '%s does not yet implement install_system_packages()' %
            __name__)

    def install_clang(self):
        raise NotImplementedError(
            'Cannot bootstrap MozillaVPN: '
            '%s does not yet implement install_clang()' %
            __name__)

    def install_qt(self):
        raise NotImplementedError(
            'Cannot bootstrap MozillaVPN: '
            '%s does not yet implement install_qt()' %
            __name__)

    def ensure_rust_modern(self):
        raise NotImplementedError(
            'Cannot bootstrap MozillaVPN: '
            '%s does not yet implement ensure_rust_modern()' %
            __name__)

    def http_download_and_save(self, url, filename, hexhash, digest='sha256'):
        f = urlopen(url)
        h = hashlib.new(digest)

        dest = os.path.join(self.mach.cache_dir, filename)
        if os.path.exists(dest):
            print(f'{filename} found in cache. Checking the hash...')
            with open(dest, 'rb') as f:
                while True:
                    data = f.read(4096)
                    if data:
                        h.update(data)
                    else:
                        break
            if h.hexdigest() == hexhash:
                return

            print('Invalid hash. Removing the file and downloading it again.')
            os.remove(dest)

        with open(dest, 'wb') as out:
            while True:
                data = f.read(4096)
                if data:
                    out.write(data)
                    h.update(data)
                else:
                    break
        if h.hexdigest() != hexhash:
            os.remove(dest)
            raise ValueError('Hash of downloaded file does not match expected hash')

    def extract_from_cache(self, filename, dirname, dest = ''):
        cachedfile = os.path.join(self.mach.cache_dir, filename)
        if not os.path.exists(cachedfile):
            raise ValueError(f'No cached file {filename}')

        dirname = os.path.join(self.mach.state_dir, dirname)
        self.remove_dir_or_file(dirname)

        if cachedfile.endswith(".tar.gz") or cachedfile.endswith(".tgz") or cachedfile.endswith(".tar.xz") or cachedfile.endswith(".tar.bz2"):
            cmd = ["tar", "xf", cachedfile]
        elif cachedfile.endswith(".zip"):
            cmd = ["unzip", "-q", cachedfile]
        else:
            raise NotImplementedError("Don't know how to unpack file: %s" % cachedfile)

        print("Unpacking %s..." % cachedfile)
        old_path = os.getcwd()
        os.chdir(self.mach.state_dir)
        with open(os.devnull, "w") as stdout:
            # These unpack commands produce a ton of output; ignore it.  The
            # .bin files are 7z archives; there's no command line flag to quiet
            # output, so we use this hammer.
            subprocess.check_call(cmd, stdout=stdout)
        print("Unpacking %s... done" % cachedfile)
        os.chdir(old_path)

        if not os.path.exists(dirname):
            raise NotImplementedError("Expected folder %s" % dirname)

        if dest != '':
            dest = os.path.join(self.mach.state_dir, dest)
            self.remove_dir_or_file(dest)
            os.rename(dirname, dest)

    def remove_dir_or_file(self, path):
        if not os.path.exists(path):
            return

        if os.path.isdir(path):
            shutil.rmtree(path)
        else:
            os.remove(path)
