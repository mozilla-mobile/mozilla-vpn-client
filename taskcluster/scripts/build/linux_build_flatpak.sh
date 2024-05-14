#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# Crudely copied from the github action.
flatpak remote-add --user --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

echo "Installing Dependencies"
mkdir fp-build-dir
flatpak-builder --user --install-deps-from=flathub --install-deps-only fp-build-dir /builds/worker/source/org.mozilla.vpn.yml

echo "Building the Flatpak"
flatpak-builder --user fp-build-dir /builds/worker/source/org.mozilla.vpn.yml

echo "Exporting the Flatpak"
mkdir fp-export-dir
flatpak build-export fp-export-dir fp-build-dir
flatpak build-bundle fp-export-dir /builds/worker/artifacts/mozillavpn.flatpak org.mozilla.vpn

echo "Exported contents:"
ls -al /builds/worker/artifacts/
