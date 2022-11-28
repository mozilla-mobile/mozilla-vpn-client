#!/bin/sh

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# switch to repository directory for setup
cd /Volumes/workspace/repository

# make sure submodules are up to date
# should already be done by Xcode cloud cloning but just to make sure
git submodule init
git submodule update

# install go, rust and camke using brew
# wget is currently not supported so we need to use brew
brew install go
brew install cmake
brew install rustup
rustup-init

# install python packages
# use --user for permissions
python3 -m pip install --upgrade pip
python3 -m pip install -r requirements.txt --user

# Get Qt and configure the build.
if [ $CI_PRODUCT_PLATFORM == 'macOS' ]
then
  # Install static Qt builds from Github
  auth_header="$(git config --local --get http.https://github.com/.extraheader)"
  git clone https://github.com/mozilla-mobile/qt_static_macos --depth 1
  cd qt_static_macos
  cat qt6* > qt_static.tar.gz
  tar xf qt_static.tar.gz
  cd ..

  # Generate the project
  mkdir build-macos
  cmake -S . -B build-macos -GXcode -DCMAKE_PREFIX_PATH=`pwd`/qt_static_macos/qt6/lib/cmake
  ln -s build-macos/Mozilla\ VPN.xcodeproj $CI_PROJECT_FILE_PATH
else
  # Install iOS Qt builds from taskcluster
  curl -L https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-ios.latest/artifacts/public%2Fbuild%2Fqt6_ios.zip --output qt_ios.zip
  unzip qt_ios.zip
  ls
  QTVERSION=$(ls qt_ios)
  echo "Using QT:$QTVERSION"

  # Generate the project
  mkdir build-ios
  qt_ios/$QTVERSION/ios/bin/qt-cmake -S . -B build-ios
  ln -s build-ios/Mozilla\ VPN.xcodeproj $CI_PROJECT_FILE_PATH
fi
