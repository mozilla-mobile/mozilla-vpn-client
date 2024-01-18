# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# This script is used in the Android Debug (universal) build task
git submodule update --init --recursive

# $1 should be the qmake arch.
# Note this is different from what aqt expects as arch:
#
# aqt-name "armv7"       -> qmake-name: "armeabi-v7a"
# aqt-name "arm64_v8a"   -> qmake-name: "arm64-v8a"
# aqt-name "x86"         -> qmake-name: "x86"
# aqt-name "x86_64"      -> qmake-name: "x86_64"

# We need to call bash with a login shell, so that conda is intitialized
source $TASK_WORKDIR/fetches/bin/activate
conda-unpack
# conda-pack add's a /activate.d/rust.sh 
# this one set's this variable to a garbage value. 
# This causes rust to fail, as it's searching an arcane linker. 
# on a "normal" env this is unset - so let's do that too.
unset CARGO_TARGET_X86_64_UNKNOWN_LINUX_GNU_LINKER
env
./scripts/android/cmake.sh -d

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
cp .tmp/src/android-build/build/outputs/apk/debug/*  /builds/worker/artifacts/

# Have nicer names :)
mv /builds/worker/artifacts/android-build-x86_64-debug.apk /builds/worker/artifacts/mozillavpn-x86_64-debug.apk
mv /builds/worker/artifacts/android-build-arm64-v8a-debug.apk /builds/worker/artifacts/mozillavpn-arm64-v8a-debug.apk
mv /builds/worker/artifacts/android-build-armeabi-v7a-debug.apk /builds/worker/artifacts/mozillavpn-armeabi-v7a-debug.apk
mv /builds/worker/artifacts/android-build-x86-debug.apk /builds/worker/artifacts//mozillavpn-x86-debug.apk

ls /builds/worker/artifacts/

# Zip up the 
(cd .tmp/src/; zip -r android-build.zip android-build)
cp .tmp/src/android-build.zip /builds/worker/artifacts/


if test -n "$(find /builds/worker/artifacts/ -maxdepth 0 -empty)" ; then
    echo "Output File not present, maybe build error?"
    exit -1
fi

ccache -s

# Check for unintended writes to the source directory.
echo "Ensuring the source dir is clean:"
./scripts/utils/dirtycheck.sh
