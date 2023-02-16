#!/bin/sh

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# switch to repository directory for setup
cd /Volumes/workspace/repository

# make sure submodules are up to date
# should already be done by XCode cloud cloning but just to make sure
git submodule update --init

# add necessary directories to path
export PATH=/Users/local/.gem/ruby/2.6.0/bin:/Users/local/Library/Python/3.8/bin:$PATH

python3 -m pip install --upgrade pip

if [ $CI_PRODUCT_PLATFORM == 'macOS' ]
then
  # generate qt_static_macos
  auth_header="$(git config --local --get http.https://github.com/.extraheader)"
  git clone https://github.com/mozilla-mobile/qt_static_macos --depth 1
  cd qt_static_macos
  cat qt6* > qt_static.tar.gz
  tar xf qt_static.tar.gz

  cat > qt6/bin/qt.conf << EOF
[Paths]
Prefix=`pwd`/qt6
EOF

  cp qt6/bin/qt.conf qt6/libexec
  cd ..

  export QT_MACOS_BIN=`pwd`/qt_static_macos/qt6/bin
  export PATH=`pwd`/qt_static_macos/qt6/bin:$PATH
else
  curl -L https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-ios.latest/artifacts/public%2Fbuild%2Fqt6_ios.zip --output qt_ios.zip
  unzip qt_ios.zip
  ls
  QTVERSION=$(ls qt_ios)
  echo "Using QT:$QTVERSION"

  export QT_IOS_BIN=/Volumes/workspace/repository/qt_ios/$QTVERSION/ios/bin
  export PATH=/Volumes/workspace/repository/qt_ios/$QTVERSION/ios/bin:/Volumes/workspace/repository/qt_ios/$QTVERSION/macos/bin:$PATH
fi

# install xcodeproj which is needed by xcode_patcher.rb
# use --user-install for permissions
gem install xcodeproj --user-install

# install python packages
# use --user for permissions
pip3 install -r requirements.txt --user

# install go and set GOROOT using brew
# wget is currently not supported so we need to use brew
brew install go

# create xcode.xconfig
cat > xcode.xconfig << EOF
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
EOF

if [ $CI_PRODUCT_PLATFORM == 'macOS' ]
then
  ./scripts/macos/apple_compile.sh macos
else
  ./scripts/macos/apple_compile.sh ios -q /Volumes/workspace/repository/qt_ios/$QTVERSION/macos/bin
fi
