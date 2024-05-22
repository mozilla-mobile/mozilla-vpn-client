#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

echo "Hello World!"
echo "Dumping the environment..."
env

echo "Dumping /etc/os-release..."
cat /etc/os-release

echo "Listing fetches..."
ls -al $MOZ_FETCHES_DIR

echo "Loading docker image..."
podman load -i $MOZ_FETCHES_DIR/image.tar.zst

echo "Building via docker image..."
podman run --rm --privileged \
    --volume=$MOZ_FETCHES_DIR:/builds/worker/source \
    --volume=$(pwd)/artifacts:/builds/worker/artifacts \
    docker.io/library/flatpak:latest /builds/worker/builder.sh

echo "Listing aftifacts..."
du h $(pwd)/artifacts
