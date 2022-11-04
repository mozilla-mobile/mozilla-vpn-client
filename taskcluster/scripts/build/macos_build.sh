#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
set -x

. $(dirname $0)/../../../scripts/utils/commons.sh

TASK_HOME=$(dirname "${MOZ_FETCHES_DIR}" )
rm -rf "${TASK_HOME}/artifacts"
mkdir -p "${TASK_HOME}/artifacts"


print N "Taskcluster macOS compilation script"
print N ""

export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8

print Y "Installing ruby dependencies..."
# use --user-install for permissions
gem install xcodeproj --user-install || die
export PATH="$HOME/.gem/ruby/2.6.0/bin:$PATH"

print Y "Installing rust..."
curl https://sh.rustup.rs -sSf | sh -s -- -y || die
export PATH="$HOME/.cargo/bin:$PATH"

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
export PYTHONIOENCODING="UTF-8"


print Y "Installing QT..."
PROJECT_HOME=`pwd`
cd ../../fetches/qt_dist || die
export QT_MACOS_BIN=`pwd`/bin
export PATH=$QT_MACOS_BIN:$PATH

cat > bin/qt.conf << EOF
[Paths]
Prefix=`pwd`
EOF
cp bin/qt.conf libexec || die
cd $PROJECT_HOME


print Y "Updating submodules..."
# should already be done by XCode cloud cloning but just to make sure
git submodule init || die
git submodule update || die

print Y "Configuring the build..."
mkdir ${MOZ_FETCHES_DIR}/build
cmake -S . -B ${MOZ_FETCHES_DIR}/build

print Y "Building the client..."
cmake --build ${MOZ_FETCHES_DIR}/build

print Y "Building the installer..."
cmake --build ${MOZ_FETCHES_DIR}/build --target pkg

print Y "Exporting the build artifacts..."
mkdir -p tmp || die
cp -r ${MOZ_FETCHES_DIR}/build/src/Mozilla\ VPN.app tmp || die
cp -r ./macos/pkg/scripts tmp || die
cp -r ./macos/pkg/Distribution tmp || die
cp -r ${MOZ_FETCHES_DIR}/macos/pkg/Resources tmp || die

print Y "Compressing the build artifacts..."
tar -C tmp -czvf "${TASK_HOME}/artifacts/MozillaVPN.tar.gz" . || die
rm -rf tmp || die

print G "Done!"
