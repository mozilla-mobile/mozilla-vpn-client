#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -x
pushd vcs
mkdir homebrew && curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew
export PATH=$PWD/homebrew/bin:$PATH

mkdir qt_dist
mkdir artifacts

brew install cmake
brew install ninja

echo "Building QT"
./scripts/utils/qt6_compile.sh $MOZ_FETCHES_DIR/qt-everywhere-src-$QT_VERSION qt_dist
echo "Build Qt- Creating dist artifact"
ls
echo $PWD
mv qt-everywhere-src-$QT_VERSION/qt_dist/* qt_dist
mkdir -p ../../public/build
zip -qr ../../public/build/qt6_mac.zip qt_dist/*
