# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# This script is used in the Android Debug (universal) build task
git submodule init
git submodule update
# glean
./scripts/utils/generate_glean.py
# translations
./scripts/utils/import_languages.py


./scripts/android/package.sh -d $QTPATH

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
ls
cp build/outputs/apk/debug/*  /builds/worker/artifacts/


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
