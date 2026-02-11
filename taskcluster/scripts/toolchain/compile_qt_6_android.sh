#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# We need to call bash with a login shell, so that conda is initialized
source $TASK_WORKDIR/fetches/conda/bin/activate
conda-unpack

# conda-pack add's a /activate.d/rust.sh
# this one set's this variable to a garbage value.
# This causes rust to fail, as it's searching an arcane linker.
# on a "normal" env this is unset - so let's do that too.
unset CARGO_TARGET_X86_64_UNKNOWN_LINUX_GNU_LINKER

echo "Environment variables:"
env | grep -E "ANDROID|QT" || true

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
QT_SOURCE_VERSION=$(echo $QT_SOURCE_DIR | awk -F"-" '{print $NF}')

# Find Qt host tools directory
QT_TOOLS_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-host-tools' | head -1)

if [[ -z "$QT_TOOLS_DIR" ]]; then
    echo "Error: Qt host tools not found in MOZ_FETCHES_DIR"
    exit 1
fi

# Apply Android API 26 patch
echo "Patching Qt for Android API 26"
patch -d ${QT_SOURCE_DIR} -p1 < ${VCS_PATH}/taskcluster/scripts/toolchain/patches/qt-android-api26-complete.patch

# Apply QTBUG patch if needed
if dpkg --compare-versions "$QT_SOURCE_VERSION" lt "6.10.3"; then
    echo "Patching for QTBUG-141830"
    patch -d ${QT_SOURCE_DIR}/qtdeclarative -p1 < ${VCS_PATH}/taskcluster/scripts/toolchain/patches/qtbug-141830-qsortfilterproxymodel.patch
fi

echo "Building Qt ${QT_SOURCE_VERSION} for Android"
echo "  Source: $QT_SOURCE_DIR"
echo "  Host tools: $QT_TOOLS_DIR"
echo "  Android SDK: $ANDROID_SDK_ROOT"
echo "  Android NDK: $ANDROID_NDK_ROOT"

mkdir -p qt-android

# Validate required environment variables
if [[ -z "$ANDROID_SDK_ROOT" ]]; then
    echo "Error: ANDROID_SDK_ROOT must be set"
    exit 1
fi

if [[ -z "$ANDROID_NDK_ROOT" ]]; then
    echo "Error: ANDROID_NDK_ROOT must be set"
    exit 1
fi

# Call the updated qt6_compile.sh with Android platform
$VCS_PATH/scripts/utils/qt6_compile.sh \
    $QT_SOURCE_DIR \
    $(pwd)/qt-android \
    --platform android \
    --qt-host-path $QT_TOOLS_DIR \
    --android-sdk $ANDROID_SDK_ROOT \
    --android-ndk $ANDROID_NDK_ROOT \
    -b $(pwd)/qt_build

echo "Build Qt - Creating dist artifact"
echo $UPLOAD_DIR
tar -cJf $UPLOAD_DIR/qt6_android.tar.xz qt-android/
