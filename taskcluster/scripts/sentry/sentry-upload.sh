#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

echo "Uploading symbol bundle"
get-secret -s project/mozillavpn/level-1/sentry -k sentry_debug_file_upload_key -f sentry_debug_file_upload_key
sentry-cli login --auth-token $(cat sentry_debug_file_upload_key)
sentry-cli debug-files upload --org mozilla -p vpn-client $(find ${MOZ_FETCHES_DIR}/MozillaVPN.dSYM/Contents/Resources/DWARF -type f)
