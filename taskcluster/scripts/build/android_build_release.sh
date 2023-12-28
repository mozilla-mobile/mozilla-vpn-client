# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# This script is used in the Android Build Release (universal) build task
git submodule update --init --recursive

# We need to call bash with a login shell, so that conda is intitialized
source $TASK_WORKDIR/fetches/bin/activate
conda-unpack
# conda-pack add's a /activate.d/rust.sh 
# this one set's this variable to a garbage value. 
# This causes rust to fail, as it's searching an arcane linker. 
# on a "normal" env this is unset - so let's do that too.
unset CARGO_TARGET_X86_64_UNKNOWN_LINUX_GNU_LINKER
env


# Get Secrets for building
if [[ "$MOZ_SCM_LEVEL" == "3" ]]; then
  echo "Fetching Tokens!"
  ./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k adjust -f adjust_token
  ./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k sentry_debug_file_upload_key -f sentry_debug_file_upload_key
else
    echo "Using dummy Tokens!"
    # write a dummy value in the files, so that we still compile sentry c:
    echo "dummy" > sentry_debug_file_upload_key
    echo "dummy" > adjust_token
fi


# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/

# $1 should be the qmake arch.
# Note this is different from what aqt expects as arch:
#
# aqt-name "armv7"       -> qmake-name: "armeabi-v7a"
# aqt-name "arm64_v8a"   -> qmake-name: "arm64-v8a"
# aqt-name "x86"         -> qmake-name: "x86"
# aqt-name "x86_64"      -> qmake-name: "x86_64"

./scripts/android/cmake.sh $QTPATH -A $1 -a $(cat adjust_token)

if [[ "$MOZ_SCM_LEVEL" == "3" ]]; then
  npm install -g @sentry/cli
  sentry-cli login --auth-token $(cat sentry_debug_file_upload_key)
  # This will ask sentry to scan all files in there and upload
  # missing debug info, for symbolification
  sentry-cli debug-files upload --org mozilla -p vpn-client --include-sources .tmp/src/android-build/build/intermediates/merged_native_libs
else
  echo "Skipping sentry stuff"
fi


# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
cp .tmp/src/android-build/build/outputs/apk/release/*  /builds/worker/artifacts/

# The Sign task will not rename them, so marking them as unsigned is a bit off. :)
mv /builds/worker/artifacts/android-build-x86_64-release-unsigned.apk /builds/worker/artifacts/mozillavpn-x86_64-release.apk
mv /builds/worker/artifacts/android-build-arm64-v8a-release-unsigned.apk /builds/worker/artifacts/mozillavpn-arm64-v8a-release.apk
mv /builds/worker/artifacts/android-build-armeabi-v7a-release-unsigned.apk /builds/worker/artifacts/mozillavpn-armeabi-v7a-release.apk
mv /builds/worker/artifacts/android-build-x86-release-unsigned.apk /builds/worker/artifacts//mozillavpn-x86-release.apk


ccache -s
