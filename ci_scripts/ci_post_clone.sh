#!/bin/sh

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# switch to repository directory for setup
cd /Volumes/workspace/repository

# make sure submodules are up to date
# should already be done by XCode cloud cloning but just to make sure
git submodule init
git submodule update

if [ $CI_PRODUCT_PLATFORM == 'macOS' ]
then
  # generate qt_static_macos
  auth_header="$(git config --local --get http.https://github.com/.extraheader)"
  git clone https://github.com/mozilla-mobile/qt_static_macos
  cd qt_static_macos
  cat x* > qt_static.tar.gz
  tar xf qt_static.tar.gz
  cd ..
  export QT_MACOS_BIN=`pwd`/qt_static_macos/qt/bin
  export PATH=`pwd`/qt_static_macos/qt/bin:$PATH
else
  # generate qt_ios
  git clone https://github.com/mozilla-mobile/qt_ios
  cd qt_ios
  cat qt5* > qt_static.tar.gz
  tar xf qt_static.tar.gz
  cd ..
  export QT_IOS_BIN=`pwd`/qt_ios/ios/bin
  export PATH=`pwd`/qt_ios/ios/bin:$PATH
fi


# add necessary directories to path
export PATH=/Users/local/.gem/ruby/2.6.0/bin:/Users/local/Library/Python/3.8/bin:$PATH

# install xcodeproj which is needed by xcode_patcher.rb
# use --user-install for permissions
gem install xcodeproj --user-install

# install python packages
# use --user for permissions
pip3 install "glean_parser==3.5" --user
pip3 install pyhumps --user
pip3 install pyyaml --user
python3 scripts/utils/generate_glean.py
python3 scripts/utils/import_languages.py -m

# install go and set GOROOT using brew
# wget is currently not supported so we need to use brew
brew install go

# create xcode.xconfig
cat > xcode.xconfig << EOF
DEVELOPMENT_TEAM = 43AQ936H96
GROUP_ID_MACOS = group.org.mozilla.macos.Guardian
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
EOF

if [ $CI_PRODUCT_PLATFORM == 'macOS' ]
then
  ./scripts/macos/apple_compile.sh macos
else
  ./scripts/macos/apple_compile.sh ios
fi

# build Qt resources
# XCode Cloud has some problem with dependencies and timing therefore we have to
# build Qt before we call xcodebuild
make -f MozillaVPN.xcodeproj/qt_makeqmake.mak
make -f MozillaVPN.xcodeproj/qt_preprocess.mak

