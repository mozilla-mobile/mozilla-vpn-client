#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

echo "Loading podman image..."
podman load -i $MOZ_FETCHES_DIR/image.tar.zst | tee /tmp/podman-load.txt
DOCKER_IMAGE_NAME=$(grep '^Loaded image' /tmp/podman-load.txt | cut -d: -f2-)

echo "Running ${DOCKER_IMAGE_NAME}..."
mkdir -p $TASK_WORKDIR/artifacts
podman run --rm --privileged \
    --volume=$MOZ_FETCHES_DIR:/mnt/source:ro \
    --volume=$TASK_WORKDIR/artifacts:/mnt/artifacts:rw \
    ${DOCKER_IMAGE_NAME} "$@"
