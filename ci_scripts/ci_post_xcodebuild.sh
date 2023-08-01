#!/bin/sh

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This is necessary in order to have sentry-cli
# install locally into the current directory
export INSTALL_DIR=$PWD

if [[ $(command -v sentry-cli) == "" ]]; then
    curl -sL https://sentry.io/get-cli/ | bash
fi

sentry-cli login --auth-token $SENTRY_UPLOAD_KEY

echo "Extracting the Symbols..."
dsymutil ${$CI_ARCHIVE_PATH}/Products/Applications/Mozilla\ VPN.app/Contents/MacOS/Mozilla\ VPN  -o MozillaVPN.dSYMs

echo "Checking & genrating a symbols bundle"
ls MozillaVPN.dSYMs/Contents/Resources/DWARF/
sentry-cli difutil check MozillaVPN.dSYMs/Contents/Resources/DWARF/*
sentry-cli difutil bundle-sources MozillaVPN.dSYMs/Contents/Resources/DWARF/*

sentry-cli --auth-token $SENTRY_UPLOAD_KEY \
    upload-dif --org mozilla \
    --include-sources . \
    --project vpn-client \
    $CI_ARCHIVE_PATH

sentry-cli debug-files upload \
  --include-sources \
  --org mozilla \
  --project vpn-client \
  MozillaVPN.dSYMs
