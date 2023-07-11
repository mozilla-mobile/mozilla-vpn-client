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

sentry-cli --auth-token $SENTRY_UPLOAD_KEY \
    upload-dif --org mozilla \
    --include-sources . \
    --project vpn-client \
    $CI_ARCHIVE_PATH

# Upload debug symbols
sentry-cli debug-files upload --auth-token $SENTRY_UPLOAD_KEY \
  --include-sources \
  --org mozilla \
  --project vpn-client \
  $CI_ARCHIVE_PATH/dSYMs
