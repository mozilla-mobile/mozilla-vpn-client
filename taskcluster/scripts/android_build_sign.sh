# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This script is used in the Android Sign (universal) build task
git submodule init
git submodule update
# glean
./scripts/generate_glean.py
# translations
echo "Importing translations"
./scripts/importLanguages.py


# Get Secrets for building
echo "Fetching Tokens!"
./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k adjust -f adjust_token

cat adjust_token
./scripts/android_package.sh $QTPATH -a $(cat adjust_token)
 
# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
cp .tmp/src/android-build/build/outputs/apk/release/*  /builds/worker/artifacts/
