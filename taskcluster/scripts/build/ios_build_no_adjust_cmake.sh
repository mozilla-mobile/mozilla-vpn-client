#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -xe


. $(dirname $0)/../../../scripts/utils/commons.sh

# Find the Output Directory and clear that
TASK_HOME=$(dirname "${MOZ_FETCHES_DIR}" )
rm -rf "${TASK_HOME}/artifacts"
mkdir -p "${TASK_HOME}/artifacts"


print N "Taskcluster iOS compilation script"
print N ""


# TC NIT: we need to assert
# that everything is UTF-8
export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8
export PYTHONIOENCODING="UTF-8"


print Y "Installing conda"
chmod +x ${MOZ_FETCHES_DIR}/miniconda.sh
bash ${MOZ_FETCHES_DIR}/miniconda.sh -b -u -p ${TASK_HOME}/miniconda
source ${TASK_HOME}/miniconda/bin/activate


print Y "Installing provided conda env..."
# TODO: Check why --force is needed if we install into TASK_HOME?
conda env create --force -f env.yml
conda activate VPN
./scripts/macos/conda_install_extras.sh  
conda info

# Should already have been done by taskcluser, but double checking c:
print Y "Get the submodules..."
git submodule update --init --recursive || die "Failed to init submodules"
for i in src/apps/*/translations/i18n; do
  git submodule update --remote $i || die "Failed to pull latest i18n from remote ($i)"
done
print G "done."

print Y "Configuring the build..."
QTVERSION=$(ls ${MOZ_FETCHES_DIR}/qt_ios)
mkdir ${MOZ_FETCHES_DIR}/build

$MOZ_FETCHES_DIR/qt_ios/$QTVERSION/ios/bin/qt-cmake -S . -B ${MOZ_FETCHES_DIR}/build -GXcode \
  -DQT_HOST_PATH="$MOZ_FETCHES_DIR/qt_ios/$QTVERSION/macos" \
  -DCMAKE_PREFIX_PATH=$MOZ_FETCHES_DIR/qt_ios/lib/cmake \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED="NO" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED="NO" \
  -DSENTRY_DSN="dummy" \
  -DSENTRY_ENVELOPE_ENDPOINT="dummy" \
  -DCMAKE_BUILD_TYPE=Release

print Y "Building the client..."
cmake --build ${MOZ_FETCHES_DIR}/build --config Release || die

print Y "Exporting the build artifacts..."
mkdir -p tmp || die
cp -r ${MOZ_FETCHES_DIR}/build/src/Release-iphoneos/* tmp || die

print Y "Compressing the build artifacts..."
tar -C tmp -czvf "${TASK_HOME}/artifacts/MozillaVPN.tar.gz" . || die
rm -rf tmp || die

# Check for unintended writes to the source directory.
print G "Ensuring the source dir is clean:"
./scripts/utils/dirtycheck.sh

print G "Done!"
