# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import math
import site
import sys

INSTALL_PYTHON_GUIDANCE_LINUX = """
See https://firefox-source-docs.mozilla.org/setup/linux_build.html#installingpython
for guidance on how to install Python on your system.
""".strip()

INSTALL_PYTHON_GUIDANCE_OSX = """
See https://firefox-source-docs.mozilla.org/setup/macos_build.html
for guidance on how to prepare your system to build Firefox. Perhaps
you need to update Xcode, or install Python using brew?
""".strip()

INSTALL_PYTHON_GUIDANCE_OTHER = """
We do not have specific instructions for your platform on how to
install Python. You may find Pyenv (https://github.com/pyenv/pyenv)
helpful, if your system package manager does not provide a way to
install a recent enough Python 3.
""".strip()

def initialize(topsrcdir):
    # Ensure we are running Python 3.6+. We run this check as soon as
    # possible to avoid a cryptic import/usage error.
    if sys.version_info < (3, 6):
        print("Python 3.6+ is required to run mach.")
        print("You are running Python", platform.python_version())
        if sys.platform.startswith("linux"):
            print(INSTALL_PYTHON_GUIDANCE_LINUX)
        elif sys.platform.startswith("darwin"):
            print(INSTALL_PYTHON_GUIDANCE_OSX)
        else:
            print(INSTALL_PYTHON_GUIDANCE_OTHER)
        sys.exit(1)

    if sys.prefix == sys.base_prefix:
        # We are not in a virtualenv. Remove global site packages
        # from sys.path.
        site_paths = set(site.getsitepackages() + [site.getusersitepackages()])
        sys.path = [path for path in sys.path if path not in site_paths]

    import machbuild.mach
    return machbuild.mach.Mach(topsrcdir)
