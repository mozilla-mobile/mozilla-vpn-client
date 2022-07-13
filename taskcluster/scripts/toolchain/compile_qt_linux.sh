#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -x
pushd vcs

mkdir qt_dist
mkdir artifacts

echo "Download QT $QT_VERSION"
curl -o qt.zip -L https://download.qt.io/archive/qt/$QT_MAJOR/$QT_VERSION/single/qt-everywhere-src-$QT_VERSION.zip
unzip -q -aa qt.zip || true

echo "Building QT"
./scripts/utils/qt6_compile.sh qt-everywhere-src-$QT_VERSION qt_dist
echo "Build Qt- Creating dist artifact"
ls
echo $PWD
mv qt-everywhere-src-$QT_VERSION/qt_dist/* qt_dist
mkdir -p ../../public/build
zip -qr ../../public/build/qt6_mac.zip qt_dist/*
