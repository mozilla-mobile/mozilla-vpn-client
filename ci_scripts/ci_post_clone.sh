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

# Install Conda and build tooling.
brew install conda

conda env create --force -f env.yml
conda activate VPN
./scripts/macos/conda_install_extras.sh  
conda info

# create xcode.xconfig
cat > xcode.xconfig << EOF
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
EOF

curl -L https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-ios.latest/artifacts/public%2Fbuild%2Fqt6_ios.zip --output qt_ios.zip
unzip -q qt_ios.zip
ls

QTVERSION=$(ls qt_ios)
echo "Using QT:$QTVERSION"
QT_IOS_PATH=$(pwd)/qt_ios/$QTVERSION/ios
QT_MACOS_PATH=$(pwd)/qt_ios/$QTVERSION/macos

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
    -e "s/\"Mozilla VPN\" VERSION/\"MozillaVPN\" VERSION/" \
    "CMakeLists.txt"

$QT_IOS_PATH/bin/qt-cmake -S . -GXcode \
  -DQT_HOST_PATH="$QT_MACOS_PATH" \
  -DCMAKE_PREFIX_PATH="$QT_IOS_PATH/lib/cmake" \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -DSENTRY_DSN=$SENTRY_DSN \
  -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
  -DCMAKE_BUILD_TYPE=Release

# Rename the default scheme to match the Xcode cloud configuration.
XCPROJ_SCHEME_DIR=MozillaVPN.xcodeproj/xcshareddata/xcschemes
mv $XCPROJ_SCHEME_DIR/mozillavpn.xcscheme $XCPROJ_SCHEME_DIR/MozillaVPN.xcscheme
