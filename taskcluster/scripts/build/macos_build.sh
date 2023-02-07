#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
set -x

. $(dirname $0)/../../../scripts/utils/commons.sh

RELEASE=1
# Parse Script arguments
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
        -d | --debug)
        RELEASE=
        shift
        ;;
    esac
done

# Find the Output Directory and clear that
TASK_HOME=$(dirname "${MOZ_FETCHES_DIR}" )
rm -rf "${TASK_HOME}/artifacts"
mkdir -p "${TASK_HOME}/artifacts"


print N "Taskcluster macOS compilation script"
print N ""


# TC NIT: we need to assert 
# that everything is UTF-8 
export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8
export PYTHONIOENCODING="UTF-8"


print Y "Installing conda environment"
export CONDA_BUILD_SYSROOT=$(xcrun --sdk macosx --show-sdk-path)
export BUILD_PREFIX=${MOZ_FETCHES_DIR}
ls -al ${MOZ_FETCHES_DIR}
source ${MOZ_FETCHES_DIR}/bin/activate
conda-unpack
conda info
#mkdir -p ${TASK_HOME}/miniconda
#tar -xzf ${MOZ_FETCHES_DIR}/conda_env_mac.tar.gz -C ${TASK_HOME}/miniconda
#source ${TASK_HOME}/miniconda/bin/activate

# Conda Cannot know installed MacOS SDK'S
# and as we use conda'provided clang/llvm
# we need to manually provide the Path. 
#
print G "Checking Available SDK'S..."
# Now you would guess the SDK path is the same on all runners
# But no. So... let's find out?
# TODO: Check if this is the same version for every runner on taskcluster .__.
export CONDA_BUILD_SYSROOT=$(xcrun --sdk macosx --show-sdk-path)
export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)


# Should already have been done by taskcluser, 
# but double checking c: 
print Y "Updating submodules..."
git submodule init || die
git submodule update || die


if [[ "$RELEASE" ]]; then
    # Install dependendy got get-secret.py 
    python3 -m pip install -r taskcluster/scripts/requirements.txt --user
    print Y "Fetching tokens..."
    # Only on a release build we have access to those secrects. 
    ./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k sentry_dsn -f sentry_dsn
    ./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k sentry_envelope_endpoint -f sentry_envelope_endpoint
    export SENTRY_ENVELOPE_ENDPOINT=$(cat sentry_envelope_endpoint)
    export SENTRY_DSN=$(cat sentry_dsn)
fi

print Y "Configuring the build..."
mkdir ${MOZ_FETCHES_DIR}/build

if [[ "$RELEASE" ]]; then
    cmake -S . -B ${MOZ_FETCHES_DIR}/build -GNinja \
        -DCMAKE_PREFIX_PATH=${MOZ_FETCHES_DIR}/qt_dist/lib/cmake \
        -DSENTRY_DSN=$SENTRY_DSN \
        -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
        -DCMAKE_BUILD_TYPE=Release \
        ${CMAKE_ARGS}
else 
    cmake -S . -B ${MOZ_FETCHES_DIR}/build -GNinja \
        -DCMAKE_PREFIX_PATH=${MOZ_FETCHES_DIR}/qt_dist/lib/cmake \
        -DCMAKE_BUILD_TYPE=Release \
        ${CMAKE_ARGS}
fi 

print Y "Building the client..."
cmake --build ${MOZ_FETCHES_DIR}/build

print Y "Building the installer..."
cmake --build ${MOZ_FETCHES_DIR}/build --target pkg

print Y "Exporting the build artifacts..."
mkdir -p tmp || die
cp -r ${MOZ_FETCHES_DIR}/build/src/Mozilla\ VPN.app tmp || die
cp -r ${MOZ_FETCHES_DIR}/build/macos/pkg/Resources tmp || die
cp -r ./macos/pkg/scripts tmp || die
cp -r ./macos/pkg/Distribution tmp || die

print Y "Compressing the build artifacts..."
tar -C tmp -czvf "${TASK_HOME}/artifacts/MozillaVPN.tar.gz" . || die
rm -rf tmp || die

print G "Done!"
