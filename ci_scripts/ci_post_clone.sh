#!/bin/sh

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# switch to repository directory for setup
cd /Volumes/workspace/repository

if [ $CI_PRODUCT_PLATFORM == 'macOS' ]
then
  echo "macOS build not supported"
  exit 1
fi

# make sure submodules are up to date
# should already be done by XCode cloud cloning but just to make sure
git submodule update --init

# add necessary directories to path
export PATH=/Users/local/Library/Python/3.8/bin:$PATH

python3 -m pip install --upgrade pip

# create xcode.xconfig
cat > xcode.xconfig << EOF
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
EOF

# TODO: REVERT HARDCODED LINK HERE
curl -L https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/Je1neDwuS6KKfuNvUrLH6g/runs/0/artifacts/public%2Fbuild%2Fqt6_ios.zip --output qt_ios.zip
unzip qt_ios.zip
ls

QTVERSION=$(ls qt_ios)
echo "Using QT:$QTVERSION"

export QT_IOS_BIN=/Volumes/workspace/repository/qt_ios/$QTVERSION/ios/bin
export PATH=/Volumes/workspace/repository/qt_ios/$QTVERSION/ios/bin:/Volumes/workspace/repository/qt_ios/$QTVERSION/macos/bin:$PATH

export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8

# Install Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source "$HOME/.cargo/env"
rustup target add aarch64-apple-ios

# Install go
brew install go

# Install Python deps
python3 -m pip install -r requirements.txt --user
export PYTHONIOENCODING="UTF-8"

# Install cmake
brew install cmake

# In previous iterations "Mozilla VPN" didn't use to have a space in it,
# to avoid the pains of migrations we will just rename here. 
# This is harmless, but still very hacky.
WORKSPACE_ROOT="$( cd "$(dirname "$0")/.." ; pwd -P )"
sed -i.bak -E \
    -e "s/Mozilla VPN/MozillaVPN/" \
    "${WORKSPACE_ROOT}/CMakeLists.txt"
sed -i.bak -E \
    -e "s/MAIN_TARGET mozillavpn/MAIN_TARGET MozillaVPN/" \
    "${WORKSPACE_ROOT}/CMakeLists.txt"

$QT_IOS_BIN/qt-cmake -S . -GXcode \
  -DQT_HOST_PATH="/Volumes/workspace/repository/qt_ios/$QTVERSION/macos" \
  -DCMAKE_PREFIX_PATH="/Volumes/workspace/repository/qt_ios/cmake" \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -DCMAKE_BUILD_TYPE=Release
