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

export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8

# make sure submodules are up to date
# should already be done by XCode cloud cloning but just to make sure
git submodule update --init

# create xcode.xconfig
cat > xcode.xconfig << EOF
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
EOF


export INDEX_KEY=mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.conda-ios-x86_64-6.6.0.latest
curl -L https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/$INDEX_KEY/artifacts/public%2Fbuild%2Fconda-ios.tar.gz --output conda-ios.tar.gz 

tar -xf conda-ios.tar.gz 
# Activate the Conda-ENV
source bin/activate
conda-unpack

# In previous iterations "Mozilla VPN" didn't use to have a space in it,
# to avoid the pains of migrations we will just rename here. 
# This is harmless, but still very hacky.
WORKSPACE_ROOT="$( cd "$(dirname "$0")/.." ; pwd -P )"
sed -i.bak -E \
    -e "s/\"Mozilla VPN\" VERSION/\"MozillaVPN\" VERSION/" \
    "CMakeLists.txt"

qt-cmake -S . -GXcode \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_ADJUST_SDK_TOKEN=$MVPN_IOS_ADJUST_TOKEN \
  -DBUILD_TESTS=OFF

# Rename the default scheme to match the Xcode cloud configuration.
XCPROJ_SCHEME_DIR=MozillaVPN.xcodeproj/xcshareddata/xcschemes
mv $XCPROJ_SCHEME_DIR/mozillavpn.xcscheme $XCPROJ_SCHEME_DIR/MozillaVPN.xcscheme
