# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This script is used in the Android Debug (universal) build task
git submodule init
git submodule update
# glean
./scripts/generate_glean.py
# translations
./scripts/importLanguages.py

# $1 should be the qmake arch. 
# Note this is different from what aqt expects as arch: 
#
# aqt-name "armv7"       -> qmake-name: "armeabi-v7a"
# aqt-name "arm64_v8a"   -> qmake-name: "arm64-v8a"
# aqt-name "x86"         -> qmake-name: "x86"
# aqt-name "x86_64"      -> qmake-name: "x86_64"
./scripts/android/package.sh -d $QTPATH -A $1

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
cp .tmp/src/android-build/mozillavpn.apk  /builds/worker/artifacts/

ccache -s
