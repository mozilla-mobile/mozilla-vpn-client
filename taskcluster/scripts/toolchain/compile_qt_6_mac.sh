#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


pushd vcs
mkdir homebrew && curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew
export PATH=$PWD/homebrew/bin:$PATH

mkdir qt_dist
mkdir artifacts

brew install cmake
brew install ninja

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
echo "Building $(basename $QT_SOURCE_DIR)"
./scripts/utils/qt6_compile.sh $QT_SOURCE_DIR $(pwd)/qt_dist

echo "Creating Qt dist artifact"
mkdir -p ../../public/build
zip -qr ../../public/build/qt6_mac.zip qt_dist/*
