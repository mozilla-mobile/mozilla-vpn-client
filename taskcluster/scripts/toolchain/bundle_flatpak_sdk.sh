#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script creates a qt-bundle that we can use in xcode-cloud and 
# in the taskcluser/ios builds ( to be coming ... )

ARTIFACT_DIR=/mnt/artifacts

# Find the app-id and manifest in the mounted source directory.
FLATPAK_APP_ID=
FLATPAK_APP_MANIFEST=
for srcfile in $(find ${VCS_PATH} -maxdepth 2 -name '*.yml') $(find ${VCS_PATH} -maxdepth 2  -name '*.yaml'); do
    if yq -e '."app-id"' $srcfile &> /dev/null; then
        FLATPAK_APP_ID=$(yq -re '."app-id"' $srcfile)
        FLATPAK_APP_MANIFEST="$srcfile"
    fi
done
if [ -z "$FLATPAK_APP_ID" ] || [ -z "$FLATPAK_APP_MANIFEST" ]; then
    echo "Failed to locate flatpak manifest in $MOZ_FETCHES_DIR" >&2
    exit 1
fi

# Setup the flathub remote.
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak remote-modify --collection-id=org.flathub.Stable flathub

# Install flatpak dependencies
flatpak-builder --install-deps-from=flathub --install-deps-only /tmp/fp-build-dir ${FLATPAK_APP_MANIFEST}

# Export the runtimes
mkdir ${TASK_WORKDIR}/flatpak-sdks
for REF in $(flatpak list --runtime --columns=ref | tail -n +1); do
    # If this runtime includes extra data - it can't be installed offline.
    if flatpak info -m ${REF} | grep -q '\[Extra Data\]'; then
        echo "Ignoring runtime ${REF} as it contains Extra Data" >&2
        continue
    fi
    flatpak create-usb ${TASK_WORKDIR}/flatpak-sdks ${REF}
done

# Compress the result.
tar -C ${TASK_WORKDIR} -cJf ${ARTIFACT_DIR}/flatpak-sdks.tar.xz flatpak-sdks
