#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Toolchain tasks need to provide a script in the taskcluster/scripts/toolchain
# directory and any kind of relative path shenanigans break the file hashing.
# I think this is really a bug in taskcluster, but let's work around it for now
# by just wrapping a call to the real podman-worker.py
$(dirname $0)/../podman-worker.py "$@"
