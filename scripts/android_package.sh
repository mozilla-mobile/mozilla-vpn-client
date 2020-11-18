#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
. $(dirname $0)/commons.sh

if [ -f .env ]; then
  . .env
fi


print N "This script compiles MozillaVPN for Android"
print N ""

if [ "$1" = "--help" ] || [ "$1" = "" ]; then
  print G "Usage:"
  print N "\t$0 <path to QT> <debug|release>"
  print N ""
  exit 0
fi
if [ "$2" = "" ]; then
  print N "No Mode Set, defaulting to -> DEBUG "
fi
if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi

if ! [ -d "$1/android/bin/" ]; then
  die "QTAndroid SDK was not found in the provided QT path"
fi
print Y "Checking Enviroment"
if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi
if ! [ -d "$1/android/bin/" ]; then
  die "QTAndroid SDK was not found in the provided QT path"
fi
if [ -z "${JAVA_HOME}" ]; then
  die "Could not find 'JAVA_HOME' in env"
fi
if [ -z "${ANDROID_NDK_ROOT}" ]; then
  die "Could not find 'ANDROID_NDK_ROOT' in env"
fi
if [ -z "${ANDROID_SDK_ROOT}" ]; then
  die "Could not find 'ANDROID_SDK_ROOT' in env"
fi

$1/android/bin/qmake -v &>/dev/null || die "qmake doesn't exist or it fails"


printn Y "Cleaning the folder... "
make distclean &>/dev/null;
print G "done."

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print Y "Importing translation files..."
python3 scripts/importLanguages.py || die "Failed to import languages"

printn Y "Computing the version... "
VERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ ).$(date +"%Y%m%d%H%M")
print G $VERSION

print Y "Configuring the android build"

cd .tmp/

if [ "$2" = "release" ]; then
  # On release builds only QT requires these *_metatypes.json
  # The files are actually all the same, but named by _ABI_ (they only differ for plattforms e.g android/ and ios/ )
  # But sometimes the resolver seems to miss the current abi and defaults to the "none" abi
  # This one was missing on my machine, let's create a "none" version in case the resolver might fail too
  printn Y "Patch qt meta data"
  cp $1/android/lib/metatypes/qt5quick_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5quick_metatypes.json
  cp $1/android/lib/metatypes/qt5charts_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5charts_metatypes.json
  cp $1/android/lib/metatypes/qt5svg_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5svg_metatypes.json
  cp $1/android/lib/metatypes/qt5widgets_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5widgets_metatypes.json
  cp $1/android/lib/metatypes/qt5gui_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5gui_metatypes.json
  cp $1/android/lib/metatypes/qt5qmlmodels_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5qmlmodels_metatypes.json
  cp $1/android/lib/metatypes/qt5qml_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5qml_metatypes.json
  cp $1/android/lib/metatypes/qt5networkauth_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5networkauth_metatypes.json
  cp $1/android/lib/metatypes/qt5network_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5network_xmetatypes.json
  cp $1/android/lib/metatypes/qt5test_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5test_metatypes.json
  cp $1/android/lib/metatypes/qt5androidextras_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5androidextras_metatypes.json
  cp $1/android/lib/metatypes/qt5core_armeabi-v7a_metatypes.json $1/android/lib/metatypes/qt5core_metatypes.json
  printn Y "Use release config"
  $1/android/bin/qmake -spec android-clang \
    CONFIG+=qtquickcompiler \
    ANDROID_ABIS="armeabi-v7a x86 arm64-v8a" \
    ..//mozillavpn.pro  || die "Qmake failed"
else
  printn Y "Use debug config \n"
  $1/android/bin/qmake -spec android-clang \
    CONFIG+=debug \
    CONFIG+=qml_debug \
    ANDROID_ABIS="armeabi-v7a" \
    ..//mozillavpn.pro || die "Qmake failed"
fi

print Y "Compiling apk_install_target in .tmp/"
make -j16 sub-src-apk_install_target || die "Compile of QT project failed"

print Y "Bundleing APK"
cd src/
make apk || die "Compile of QT project failed"
print G "All done!"

print N "Your .APK is Located in .tmp/src/android-build/mozillavpn.apk"
