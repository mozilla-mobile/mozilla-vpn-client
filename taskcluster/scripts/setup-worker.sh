#!/bin/sh
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mkdir -p /builds/worker
mkdir /builds/worker/artifacts

# Setup the worker account for docker images
if getent passwd 1000; then
    # Rename and modify an existing user
    PREV_USER_NAME=$(getent passwd 1000 | cut -d: -f1)
    usermod -l worker ${PREV_USER_NAME}
    groupmod -n worker ${PREV_USER_NAME}
    usermod -d /builds/worker -s /bin/bash -m worker
else
   # Create a new worker account
   useradd -d /builds/worker -s /bin/bash -m worker -u 1000
fi
chown -R worker:worker /builds/worker
