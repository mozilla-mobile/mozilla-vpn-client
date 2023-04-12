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
for qttool in $(find $(pwd)/qt_dist/bin -executable -type f); do
    ldd $qttool | grep '=>' | awk '{print $3}' >> qtlibdeps.txt
done
cp -dv $(sort -u qtlibdeps.txt) $(pwd)/qt_dist/lib/

echo "Build Qt- Creating dist artifact"
ls
echo $UPLOAD_DIR
tar -cJf $UPLOAD_DIR/qt6_linux.tar.xz qt_dist/
