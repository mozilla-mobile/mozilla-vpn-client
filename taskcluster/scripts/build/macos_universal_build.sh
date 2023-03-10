#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e


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

print Y "Installing rust..."
curl https://sh.rustup.rs -sSf | sh -s -- -y || die
export PATH="$HOME/.cargo/bin:$PATH"

print Y "Installing homebrew, cmake, ninja..."
mkdir homebrew && curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew
export PATH=$PWD/homebrew/bin:$PATH
brew install cmake
brew install ninja

print Y "Installing go..."
curl -O https://dl.google.com/go/go1.17.6.darwin-amd64.tar.gz
tar -xzf go1.17.6.darwin-amd64.tar.gz
export PATH="`pwd`/go/bin:$PATH"

print Y "Installing python dependencies..."
# use --user for permissions
python3 -m pip install -r requirements.txt --user

# Should already have been done by taskcluser, but double checking c:
print Y "Get the submodules..."
git submodule update --init --depth 1 || die "Failed to init submodules"
git submodule update --remote i18n || die "Failed to pull latest i18n from remote"
print G "done."

# write a dummy value in the files, so that we still compile sentry c:
echo "dummy" > sentry_dsn
echo "dummy" > sentry_envelope_endpoint
echo "dummy" > sentry_debug_file_upload_key

export SENTRY_ENVELOPE_ENDPOINT=$(cat sentry_envelope_endpoint)
export SENTRY_DSN=$(cat sentry_dsn)
#Install Sentry CLI, if' not already installed from previous run.
if ! command -v sentry-cli &> /dev/null
then
    export PATH="$HOME/node/bin:$PATH"
    npm config set prefix=$HOME/node
    npm install @sentry/cli
fi


print Y "Configuring the build..."
mkdir ${MOZ_FETCHES_DIR}/build

cmake -S . -B ${MOZ_FETCHES_DIR}/build -GNinja \
        -DCMAKE_PREFIX_PATH=${MOZ_FETCHES_DIR}/qt_dist/lib/cmake \
        -DSENTRY_DSN=$SENTRY_DSN \
        -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
        -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
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
    sentry-cli login --auth-token $(cat sentry_debug_file_upload_key)
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
