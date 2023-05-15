#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

echo "Download QT $QT_VERSION"
curl -o qt.tar.xz -L https://download.qt.io/archive/qt/$QT_MAJOR/$QT_VERSION/single/qt-everywhere-src-$QT_VERSION.tar.xz
tar -xf qt.tar.xz

echo "Building QT"
mkdir qt_dist
./vcs/scripts/utils/qt6_compile.sh qt-everywhere-src-$QT_VERSION $(pwd)/qt_dist

echo "Bundling extra libs"
SYSTEM_LIB_DIR="/usr/lib/$(dpkg-architecture -q DEB_BUILD_MULTIARCH)"
cp -dv $(find ${SYSTEM_LIB_DIR} -name 'libicu*') qt_dist/lib/

echo "Build Qt- Creating dist artifact"
ls
echo $UPLOAD_DIR
tar -cJf $UPLOAD_DIR/qt6_linux.tar.xz qt_dist/
