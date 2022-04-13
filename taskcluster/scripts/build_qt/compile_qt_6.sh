#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


mkdir homebrew && curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew
export PATH=$PWD/homebrew/bin:$PATH

mkdir qt_dist
mkdir artifacts

brew install cmake
brew install ninja

echo "Download QT $QT_VERSION"
curl -o qt.zip -L https://download.qt.io/archive/qt/$QT_MAJOR/$QT_VERSION/single/qt-everywhere-src-$QT_VERSION.zip
unzip -q -aa qt.zip || true 

echo "Building QT"
./scripts/utils/qt6_compile.sh qt-everywhere-src-$QT_VERSION qt_dist 
echo "Build Qt- Creating dist artifact"
ls 
echo $PWD
zip -r artifacts/qt6_mac.zip qt-everywhere-src-$QT_VERSION/qt_dist/*