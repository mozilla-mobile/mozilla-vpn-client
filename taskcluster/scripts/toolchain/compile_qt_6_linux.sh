#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
cd $HOME

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
QT_SOURCE_VERSION=$(echo $QT_SOURCE_DIR | awk -F"-" '{print $NF}')

if dpkg --compare-versions "$QT_SOURCE_VERSION" lt "6.10.3"; then
    echo "Patching for QTBUG-141830"
    patch -d ${QT_SOURCE_DIR}/qtdeclarative -p1 < ${VCS_PATH}/taskcluster/scripts/toolchain/patches/qtbug-141830-qsortfilterproxymodel.patch
fi

echo "Building $(basename $QT_SOURCE_DIR)"
mkdir qt-linux

$VCS_PATH/scripts/utils/qt6_compile.sh $QT_SOURCE_DIR $(pwd)/qt-linux -b $(pwd)/qt_build

echo "Patch Qt configuration"
cat << EOF > $(pwd)/qt-linux/bin/qt.conf
[Paths]
Prefix=..
EOF

cat << EOF > $(pwd)/qt-linux/libexec/qt.conf
[Paths]
Prefix=..
EOF

echo "Bundling extra libs"
for qttool in $(find $(pwd)/qt-linux/bin -executable -type f); do
    ldd $qttool | grep '=>' | awk '{print $3}' >> $(pwd)/qt_build/qtlibdeps.txt
done
for qtlibdep in $(sort -u $(pwd)/qt_build/qtlibdeps.txt); do
    cp -v $qtlibdep $(pwd)/qt-linux/lib/
    patchelf --set-rpath '$ORIGIN/../lib' $(pwd)/qt-linux/lib/$(basename $qtlibdep) 
done

echo "Build Qt- Creating dist artifact"
echo $UPLOAD_DIR
tar -cJf $UPLOAD_DIR/qt6_linux.tar.xz qt-linux/
