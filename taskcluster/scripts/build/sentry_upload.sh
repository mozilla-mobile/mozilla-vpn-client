# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# Only on a release build we have access to those secrects.
if [ "${MOZ_SCM_LEVEL}" -ge "3" ]; then
    get-secret -s project/mozillavpn/level-1/sentry -k sentry_debug_file_upload_key -f sentry_debug_file_upload_key
else
    echo "dummy" > sentry_debug_file_upload_key
fi

echo "Listing env..."
env

echo ""
echo "Listing ${MOZ_FETCHES_DIR}..."
ls -al ${MOZ_FETCHES_DIR}
