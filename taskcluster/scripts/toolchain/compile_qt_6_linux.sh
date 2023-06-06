#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
echo "Building $(basename $QT_SOURCE_DIR)"
mkdir qt_dist
./vcs/scripts/utils/qt6_compile.sh $QT_SOURCE_DIR $(pwd)/qt_dist

echo "Patch Qt configuration"
cat << EOF > $(pwd)/qt_dist/bin/qt.conf
[Paths]
Prefix=..
EOF

cat << EOF > $(pwd)/qt_dist/libexec/qt.conf
[Paths]
Prefix=..
EOF

echo "Bundling extra libs"
for qttool in $(find $(pwd)/qt_dist/bin -executable -type f); do
    ldd $qttool | grep '=>' | awk '{print $3}' >> qtlibdeps.txt
done
for qtlibdep in $(sort -u qtlibdeps.txt); do
    cp -v $qtlibdep $(pwd)/qt_dist/lib/
    patchelf --set-rpath '$ORIGIN/../lib' $(pwd)/qt_dist/lib/$(basename $qtlibdep) 
done

echo "Build Qt- Creating dist artifact"
ls
echo $UPLOAD_DIR
tar -cJf $UPLOAD_DIR/qt6_linux.tar.xz qt_dist/
