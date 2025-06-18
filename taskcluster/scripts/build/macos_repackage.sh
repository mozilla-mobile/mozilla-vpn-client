#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

. $(dirname $0)/../../../scripts/utils/commons.sh

# For some diagnosis
print N "Taskcluster macOS repackage script"
print N ""

# Add CMake to the path
CMAKE_APP_BUNDLE=$(find ${MOZ_FETCHES_DIR} -name 'CMake.app')
export PATH=${PATH}:${CMAKE_APP_BUNDLE}/Contents/bin

# Find the Output Directory and clear that
TASK_HOME=$(dirname "${MOZ_FETCHES_DIR}" )
rm -rf "${TASK_HOME}/artifacts"
mkdir -p "${TASK_HOME}/artifacts"

# Extract the signed macOS application bundle.
tar -C ${TASK_HOME} -xzvf ${MOZ_FETCHES_DIR}/MozillaVPN.tar.gz
ls -al ${TASK_HOME}

# Build the macOS installer package
print Y "Building the installer..."
mkdir ${TASK_HOME}/build-macpkg
cmake -S macos/pkg -B ${TASK_HOME}/build-macpkg \
    -DMACPKG_SOURCE_BUNDLE="${TASK_HOME}/Mozilla VPN.app"
cmake --build ${TASK_HOME}/build-macpkg --target macpkg

# Output the macOS installer package
cp ${TASK_HOME}/build-macpkg/MozillaVPN.pkg ${TASK_HOME}/artifacts || die
print G "Done!"
