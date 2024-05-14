#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

SOURCE_DIR=/mnt/source
ARTIFACT_DIR=/mnt/artifacts

# Find the app-id and manifest in the mounted source directory.
FLATPAK_APP_ID=
FLATPAK_APP_MANIFEST=
for srcfile in $(find ${SOURCE_DIR} -name '*.yml') $(find ${SOURCE_DIR} -name '*.yaml'); do
    if yq -e '."app-id"' $srcfile &> /dev/null; then
        FLATPAK_APP_ID=$(yq -re '."app-id"' $srcfile)
        FLATPAK_APP_MANIFEST="$srcfile"
    fi
done
if [ -z "$FLATPAK_APP_ID" ] || [ -z "$FLATPAK_APP_MANIFEST" ]; then
    echo "Failed to locate flatpak manifest in $SOURCE_DIR" >&2
    exit 1
fi

# Crudely copied from the github action.
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

echo "Installing Dependencies"
mkdir fp-build-dir
flatpak-builder --install-deps-from=flathub --install-deps-only fp-build-dir ${FLATPAK_APP_MANIFEST}

echo "Building the Flatpak"
flatpak-builder fp-build-dir ${FLATPAK_APP_MANIFEST}

echo "Exporting the Flatpak"
mkdir ${ARTIFACT_DIR}/exports
flatpak build-export ${ARTIFACT_DIR}/exports fp-build-dir
flatpak build-bundle ${ARTIFACT_DIR}/exports ${ARTIFACT_DIR}/${FLATPAK_APP_ID}.flatpak ${FLATPAK_APP_ID}
