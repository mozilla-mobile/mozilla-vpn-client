#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)

# Start Wine 
wineserver -p 
wine64 wineboot 
service winbind start

$MOZ_FETCHES_DIR/opt/msvc/install.sh $MOZ_FETCHES_DIR/opt/msvc
rm lowercase fixinclude install.sh vsdownload.py 
rm -rf wrappers 

# Install the Linux Version of QT, as we are Cross-Compiling
python3 -m pip install aqtinstall
python3 -m aqt install-qt -O /opt/Qt_Linux linux desktop $QT_VERSION -m all 

#cd $workdir
export PATH=$MOZ_FETCHES_DIR/opt/msvc/bin/x64:$PATH
export CC=cl 
export CXX=cl
export CMAKE_SYSTEM_NAME="Windows"
export QT_HOST_PATH="/opt/Qt_Linux/$QT_VERSION/gcc_64/"
export CMAKE_TOOLCHAIN_FILE="/opt/wine_cross.toolchain"

source $MOZ_FETCHES_DIR/opt/msvc/bin/x64/msvcenv.sh 

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

echo "Build Qt- Creating dist artifact"
ls
echo $UPLOAD_DIR
tar -cJf $UPLOAD_DIR/qt6_windows_cross.tar.xz qt_dist/
