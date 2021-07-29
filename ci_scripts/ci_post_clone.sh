#!/bin/sh

#  ci_post_clone.sh
#  MozillaVPN
#
#  Created by mozilla on 29.07.21.
#  

cd /Volumes/workspace/repository

curl -L https://download.qt.io/archive/qt/5.15/5.15.1/single/qt-everywhere-src-5.15.1.tar.xz --output qt-everywhere-src-5.15.1.tar.xz
tar vxf qt-everywhere-src-5.15.1.tar.xz
mv qt-everywhere-src-5.15.1 qt
./qt/configure -xplatform macx-ios-clang -release
bash scripts/qt5_compile.sh `pwd`/qt qt
export QT_IOS_BIN=`pwd`/qt/qt/bin

./scripts/apple_compile.sh ios
