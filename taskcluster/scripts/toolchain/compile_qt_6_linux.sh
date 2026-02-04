#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
cd $HOME

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
QT_SOURCE_VERSION=$(echo $QT_SOURCE_DIR | awk -F"-" '{print $NF}')

if [[ $(echo -e "${QT_SOURCE_VERSION}\n6.10.3" | sort --version-sort | head -1) != "6.10.3" ]]; then
    echo "Patching for QTBUG-141830"
    patch -d ${QT_SOURCE_DIR}/qtdeclarative -p1 < ${VCS_PATH}/taskcluster/scripts/toolchain/patches/qtbug-141830-qsortfilterproxymodel.patch
fi

echo "Installing Qt build dependencies"
if [ -f /etc/redhat-release ]; then
    sudo yum -y install \
            at-spi2-core-devel \
            clang \
            clang-devel \
            openssl-static
elif [ -f /etc/debian_version ]; then
    sudo apt-get -y install \
            libatspi2.0-dev \
            libdbus-1-dev \
            libfontconfig1-dev \
            libfreetype6-dev \
            libssl-dev \
            libx11-dev \
            libx11-xcb-dev \
            libxext-dev \
            libxfixes-dev \
            libxi-dev \
            libxrender-dev \
            libxcb1-dev \
            libxcb-cursor-dev \
            libxcb-glx0-dev \
            libxcb-keysyms1-dev \
            libxcb-image0-dev \
            libxcb-shm0-dev \
            libxcb-icccm4-dev \
            libxcb-sync-dev \
            libxcb-xfixes0-dev \
            libxcb-shape0-dev \
            libxcb-randr0-dev \
            libxcb-render-util0-dev \
            libxcb-util-dev \
            libxcb-xinerama0-dev \
            libxcb-xkb-dev \
            libxkbcommon-dev \
            libxkbcommon-x11-dev
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
