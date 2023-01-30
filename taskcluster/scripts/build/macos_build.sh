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


print Y "Installing conda"
chmod +x ${MOZ_FETCHES_DIR}/miniconda.sh
bash ${MOZ_FETCHES_DIR}/miniconda.sh -b -u -p ${TASK_HOME}/miniconda
source ${TASK_HOME}/miniconda/bin/activate


print Y "Installing provided conda env..."
# TODO: Check why --force is needed if we install into TASK_HOME?
conda env create --force -f env.yml       
conda activate VPN         
conda info 

# Conda Cannot know installed MacOS SDK'S
# and as we use conda'provided clang/llvm
# we need to manually provide the Path. 
#
print G "Checking Available SDK'S..."
# Now you would guess the SDK path is the same on all runners
# But no. So... let's find out?
# TODO: Check if this is the same version for every runner on taskcluster .__.
export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)


# Should already have been done by taskcluser, 
# but double checking c: 
print Y "Updating submodules..."
git submodule init || die
git submodule update || die

# Install dependendy got get-secret.py 
python3 -m pip install -r taskcluster/scripts/requirements.txt
print Y "Fetching tokens..."
# Only on a release build we have access to those secrects. 
./taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/sentry -k sentry_dsn -f sentry_dsn
./taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/sentry -k sentry_envelope_endpoint -f sentry_envelope_endpoint
./taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/sentry -k sentry_debug_file_upload_key -f sentry_debug_file_upload_key
export SENTRY_ENVELOPE_ENDPOINT=$(cat sentry_envelope_endpoint)
export SENTRY_DSN=$(cat sentry_dsn)
#Install Sentry CLI, if' not already installed from previous run. 
if ! command -v sentry-cli &> /dev/null
then
    npm install -g @sentry/cli
fi
sentry-cli login --auth-token $(cat sentry_debug_file_upload_key)


print Y "Configuring the build..."
mkdir ${MOZ_FETCHES_DIR}/build

cmake -S . -B ${MOZ_FETCHES_DIR}/build -GNinja \
        -DCMAKE_PREFIX_PATH=${MOZ_FETCHES_DIR}/qt_dist/lib/cmake \
        -DSENTRY_DSN=$SENTRY_DSN \
        -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo


print Y "Building the client..."
cmake --build ${MOZ_FETCHES_DIR}/build

print Y "Building the installer..."
cmake --build ${MOZ_FETCHES_DIR}/build --target pkg

print Y "Exporting the build artifacts..."
mkdir -p tmp || die


print Y "Extracting the Symbols..."
dsymutil ${MOZ_FETCHES_DIR}/build/src/Mozilla\ VPN.app/Contents/MacOS/Mozilla\ VPN  -o tmp/MozillaVPN.dsym


print Y "Checking & genrating a symbols bundle"
ls tmp/MozillaVPN.dsym/Contents/Resources/DWARF/
sentry-cli difutil check tmp/MozillaVPN.dsym/Contents/Resources/DWARF/*
sentry-cli difutil bundle-sources tmp/MozillaVPN.dsym/Contents/Resources/DWARF/*

if [[ "$RELEASE" ]]; then      
    print Y "Uploading the Symbols..." 
    sentry-cli debug-files upload --org mozilla -p vpn-client tmp/MozillaVPN.dsym/Contents/Resources/DWARF/*
fi

cp -r ${MOZ_FETCHES_DIR}/build/src/Mozilla\ VPN.app tmp || die
cp -r ${MOZ_FETCHES_DIR}/build/macos/pkg/Resources tmp || die
cp -r ./macos/pkg/scripts tmp || die
cp -r ./macos/pkg/Distribution tmp || die

print Y "Compressing the build artifacts..."
tar -C tmp -czvf "${TASK_HOME}/artifacts/MozillaVPN.tar.gz" . || die
rm -rf tmp || die

print G "Done!"
