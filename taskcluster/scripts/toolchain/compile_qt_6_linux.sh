#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
cd $HOME

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
QT_SOURCE_VERSION=$(echo $QT_SOURCE_DIR | awk -F"-" '{print $NF}')

if dpkg --compare-versions "$QT_SOURCE_VERSION" lt "6.4.2"; then
    echo "Patching for QTBUG-109046"
    patch -d ${QT_SOURCE_DIR}/qtwebengine -p1 << EOF
From 240e71877865ed07e4c8d5bd4553aa0772c2adf4 Mon Sep 17 00:00:00 2001
From: Alexey Edelev <alexey.edelev@qt.io>
Date: Wed, 23 Nov 2022 12:40:45 +0100
Subject: [PATCH] Fix Linux build with CMake versions >= 3.25
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit

The 'LINUX' variable exists in CMake since the version 3.25. This
variable previously was undefined while preparsing the configure.cmake
files. Since the CMake script that defines the 'check_for_ulimit'
function is not included while evaluating configure.cmake first time
we need to add a stub.

Pick-to: 6.2 6.4
Change-Id: I25bdec4f4a1b6af23174507a8f0f9cbf01f0c398
Reviewed-by: Jörg Bornemann <joerg.bornemann@qt.io>
---
 configure.cmake | 2 ++
 1 file changed, 2 insertions(+)

diff --git a/configure.cmake b/configure.cmake
index 37ac1f42846..a082ec1a665 100644
--- a/configure.cmake
+++ b/configure.cmake
@@ -6,6 +6,8 @@ if(QT_CONFIGURE_RUNNING)
     endfunction()
     function(add_check_for_support)
     endfunction()
+    function(check_for_ulimit)
+    endfunction()
 else()
     find_package(Ninja 1.7.2)
     find_package(Gn \${QT_REPO_MODULE_VERSION} EXACT)
EOF
fi

echo "Building $(basename $QT_SOURCE_DIR)"
mkdir qt_dist

$VCS_PATH/scripts/utils/qt6_compile.sh $QT_SOURCE_DIR $(pwd)/qt_dist -b $(pwd)/qt_build

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
    ldd $qttool | grep '=>' | awk '{print $3}' >> $(pwd)/qt_build/qtlibdeps.txt
done
for qtlibdep in $(sort -u $(pwd)/qt_build/qtlibdeps.txt); do
    cp -v $qtlibdep $(pwd)/qt_dist/lib/
    patchelf --set-rpath '$ORIGIN/../lib' $(pwd)/qt_dist/lib/$(basename $qtlibdep) 
done

echo "Build Qt- Creating dist artifact"
echo $UPLOAD_DIR
tar -cJf $UPLOAD_DIR/qt6_linux.tar.xz qt_dist/
