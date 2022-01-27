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

./scripts/android_package.sh -d $QTPATH

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/
cp .tmp/src/android-build/mozillavpn.apk  /builds/worker/artifacts/
