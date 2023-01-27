#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi

print Y "Copy and patch Adjust SDK..."
mkdir -p android/adjust/src/
cp -a "3rdparty/adjust-android-sdk/Adjust/sdk-core/src/main/java/com/." "android/adjust/src/com" || die "Failed to copy the adjust codebase"
git apply --directory="android/adjust/src" "3rdparty/adjust_https_to_http.diff" || die "Failed to apply the adjust http patch"

print G "Copy and patch Adjust SDK..."