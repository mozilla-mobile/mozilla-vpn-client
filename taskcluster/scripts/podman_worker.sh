#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# Create a temp dir for podman artifacts.
PODMAN_TEMP_DIR=$(mktemp -d)
cleanup() {
  rm -rf $PODMAN_TEMP_DIR
}
trap cleanup INT ABRT TERM EXIT

# Load the image.
echo "Loading podman image..."
podman load -i $MOZ_FETCHES_DIR/image.tar.zst | tee ${PODMAN_TEMP_DIR}/podman-load.txt
PODMAN_IMAGE_NAME=$(grep '^Loaded image' ${PODMAN_TEMP_DIR}/podman-load.txt | cut -d: -f2- | tr -d '[:space:]')

# Prepare the image environment.
# Nice-to-have: inherit the env from the taskcluster payload.
cat << EOF > $PODMAN_TEMP_DIR/podman-env.txt
BUILD_TIMESTAMP=${BUILD_TIMESTAMP}
TASK_OWNER=${TASK_OWNER}
TASK_ID=${TASK_ID}
MOZ_SCM_LEVEL=${MOZ_SCM_LEVEL}
MOZ_AUTOMATION=${MOZ_AUTOMATION}
MOZ_FETCHES_DIR=/mnt/source
EOF

# Run the image.
echo "Running ${PODMAN_IMAGE_NAME}..."
mkdir -p $TASK_WORKDIR/artifacts
podman run --rm --privileged \
    --env-file $PODMAN_TEMP_DIR/podman-env.txt \
    --volume=$MOZ_FETCHES_DIR:/mnt/source:ro \
    --volume=$TASK_WORKDIR/artifacts:/mnt/artifacts:rw \
    ${PODMAN_IMAGE_NAME} "$@"
