#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -xe


. $(dirname $0)/../../../scripts/utils/commons.sh

print N "Taskcluster iOS compilation script"
print N ""

export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8

print Y "Installing rust..."
curl https://sh.rustup.rs -sSf | sh -s -- -y || die
export PATH="$HOME/.cargo/bin:$PATH"
rustup target add aarch64-apple-ios

print Y "Installing go..."
curl -O https://dl.google.com/go/go1.18.10.darwin-amd64.tar.gz
tar -xzf go1.18.10.darwin-amd64.tar.gz
export PATH="`pwd`/go/bin:$PATH"

print Y "Installing python dependencies..."
# use --user for permissions
python3 -m pip install -r requirements.txt --user
export PYTHONIOENCODING="UTF-8"

print Y "Installing QT..."
PROJECT_HOME=`pwd`
QTVERSION=$(ls ${MOZ_FETCHES_DIR}/qt_ios)
echo "Using QT:$QTVERSION"

print Y "Installing Homebrew..."
# We have to install it locally like this because of Taskcluster permission policies.
mkdir homebrew && curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew
export PATH=$PATH:$(pwd)/homebrew/bin

print Y "Installing cmake..."
brew install cmake

export PATH=$PATH:$GOROOT/bin

print Y "Get the submodules..."
git submodule update --init --depth 1 || die "Failed to init submodules"
for i in src/apps/*/translations/i18n; do
  git submodule update --remote $i || die "Failed to pull latest i18n from remote ($i)"
done
print G "done."

print Y "Configuring the build..."

# create xcode.xconfig
cat > xcode.xconfig << EOF
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
EOF

print Y "Building xcodeproj..."
$MOZ_FETCHES_DIR/qt_ios/$QTVERSION/ios/bin/qt-cmake -S . -B $MOZ_FETCHES_DIR/build -GXcode \
  -DQT_HOST_PATH="$MOZ_FETCHES_DIR/qt_ios/$QTVERSION/macos" \
  -DCMAKE_PREFIX_PATH=$MOZ_FETCHES_DIR/qt_ios/lib/cmake \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED="NO" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED="NO" \

print Y "Compiling..."
cmake --build build -j$(nproc) || die

print Y "Exporting the artifact..."
mkdir -p tmp || die
cp -r Release-iphoneos/* tmp || die
cd tmp || die

# From checkout dir to actual task base directory
TASK_HOME=$(dirname "${MOZ_FETCHES_DIR}" )
rm -rf "${TASK_HOME}/artifacts"
mkdir -p "${TASK_HOME}/artifacts"
tar -czvf "${TASK_HOME}/artifacts/MozillaVPN.tar.gz" . || die
cd .. || die
rm -rf tmp || die

print G "Done!"
