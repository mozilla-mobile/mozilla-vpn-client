#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# Find the app-id and manifest in the mounted source directory.
FLATPAK_APP_ID=
FLATPAK_APP_MANIFEST=
for srcfile in $(find ${MOZ_FETCHES_DIR} -name '*.yml') $(find ${MOZ_FETCHES_DIR} -name '*.yaml'); do
    if yq -e '."app-id"' $srcfile &> /dev/null; then
        FLATPAK_APP_ID=$(yq -re '."app-id"' $srcfile)
        FLATPAK_APP_MANIFEST="$srcfile"
    fi
done
if [ -z "$FLATPAK_APP_ID" ] || [ -z "$FLATPAK_APP_MANIFEST" ]; then
    echo "Failed to locate flatpak manifest in $MOZ_FETCHES_DIR" >&2
    exit 1
fi

# Setup the remotes.
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak remote-modify --collection-id=org.flathub.Stable flathub

echo "Installing Dependencies"
FLAPTAK_SIDELOAD=$(find ${MOZ_FETCHES_DIR} -type d -name '.ostree' -printf '%p/repo\n')
if [ -d "${FLAPTAK_SIDELOAD}" ]; then
    # Install all packages found in the sideload repo.
    flatpak install --sideload-repo=${FLAPTAK_SIDELOAD} -y $(flatpak remote-ls file://${FLAPTAK_SIDELOAD} --columns=ref | tail -n +1)
else
    # Download from flathub
    flatpak-builder --install-deps-from=flathub --install-deps-only /tmp/fp-build-dir ${FLATPAK_APP_MANIFEST}
fi

echo "Building the Flatpak"
flatpak-builder fp-build-dir ${FLATPAK_APP_MANIFEST}

echo "Exporting the Flatpak"
mkdir fp-export-dir
flatpak build-export fp-export-dir fp-build-dir
flatpak build-bundle fp-export-dir ${UPLOAD_DIR}/${FLATPAK_APP_ID}.flatpak ${FLATPAK_APP_ID}
