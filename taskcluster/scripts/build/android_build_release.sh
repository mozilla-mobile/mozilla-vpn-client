# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This script is used in the Android Build Release (universal) build task
git submodule init
git submodule update
# translations
echo "Importing translations"
./scripts/utils/import_languages.py

# Get Secrets for building
echo "Fetching Tokens!"
./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k adjust -f adjust_token

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/

# $1 should be the qmake arch. 
# Note this is different from what aqt expects as arch: 
#
# aqt-name "armv7"       -> qmake-name: "armeabi-v7a"
# aqt-name "arm64_v8a"   -> qmake-name: "arm64-v8a"
# aqt-name "x86"         -> qmake-name: "x86"
# aqt-name "x86_64"      -> qmake-name: "x86_64"
./scripts/android/package.sh $QTPATH -A $1 -a $(cat adjust_token)

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
cp .tmp/src/android-build/build/outputs/apk/release/*  /builds/worker/artifacts/

# The Sign task will not rename them, so marking them as unsigned is a bit off. :) 
mv /builds/worker/artifacts/android-build-x86_64-release-unsigned.apk /builds/worker/artifacts/mozillavpn-x86_64-release.apk
mv /builds/worker/artifacts/android-build-arm64-v8a-release-unsigned.apk /builds/worker/artifacts/mozillavpn-arm64-v8a-release.apk
mv /builds/worker/artifacts/android-build-armeabi-v7a-release-unsigned.apk /builds/worker/artifacts/mozillavpn-armeabi-v7a-release.apk
mv /builds/worker/artifacts/android-build-x86-release-unsigned.apk /builds/worker/artifacts//mozillavpn-x86-release.apk


ccache -s

