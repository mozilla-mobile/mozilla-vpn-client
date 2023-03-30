# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# This script is used in the Android Debug (universal) build task
git submodule update --init --depth 1

for i in src/apps/*/translations/i18n; do
  git submodule update --remote $i
done

# $1 should be the qmake arch.
# Note this is different from what aqt expects as arch:
#
# aqt-name "armv7"       -> qmake-name: "armeabi-v7a"
# aqt-name "arm64_v8a"   -> qmake-name: "arm64-v8a"
# aqt-name "x86"         -> qmake-name: "x86"
# aqt-name "x86_64"      -> qmake-name: "x86_64"
./scripts/android/cmake.sh -d $QTPATH -A $1

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
cp .tmp/src/apps/vpn/android-build/build/outputs/apk/debug/*  /builds/worker/artifacts/

# Have nicer names :)
mv /builds/worker/artifacts/android-build-x86_64-debug.apk /builds/worker/artifacts/mozillavpn-x86_64-debug.apk
mv /builds/worker/artifacts/android-build-arm64-v8a-debug.apk /builds/worker/artifacts/mozillavpn-arm64-v8a-debug.apk
mv /builds/worker/artifacts/android-build-armeabi-v7a-debug.apk /builds/worker/artifacts/mozillavpn-armeabi-v7a-debug.apk
mv /builds/worker/artifacts/android-build-x86-debug.apk /builds/worker/artifacts//mozillavpn-x86-debug.apk

ls /builds/worker/artifacts/

if test -n "$(find /builds/worker/artifacts/ -maxdepth 0 -empty)" ; then
    echo "Output File not present, maybe build error?"
    exit -1
fi

ccache -s
