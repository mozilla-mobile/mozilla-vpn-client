# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# This script is used in the Android Build Release (universal) build task
git submodule init
git submodule update
# glean
./scripts/utils/generate_glean.py
# translations
echo "Importing translations"
./scripts/utils/import_languages.py

# Get Secrets for building
echo "Fetching Tokens!"
./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k adjust -f adjust_token

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/

./scripts/android/package.sh $QTPATH -a $(cat adjust_token)

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
ls
cp build/outputs/apk/release/*  /builds/worker/artifacts/

# The Sign task will not rename them, so marking them as unsigned is a bit off. :) 
mv /builds/worker/artifacts/android-build-x86_64-release-unsigned.apk /builds/worker/artifacts/mozillavpn-x86_64-release.apk
mv /builds/worker/artifacts/android-build-arm64-v8a-release-unsigned.apk /builds/worker/artifacts/mozillavpn-arm64-v8a-release.apk
mv /builds/worker/artifacts/android-build-armeabi-v7a-release-unsigned.apk /builds/worker/artifacts/mozillavpn-armeabi-v7a-release.apk
mv /builds/worker/artifacts/android-build-x86-release-unsigned.apk /builds/worker/artifacts//mozillavpn-x86-release.apk


ccache -s

