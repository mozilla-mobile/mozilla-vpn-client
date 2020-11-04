#!/bin/bash

. $(dirname $0)/commons.sh

if [ -f .env ]; then
  . .env
fi
# export ANDROID_NDK_ROOT = /Users/mozilla/Library/Android/sdk/ndk/21.1.6352462


print N "This script compiles MozillaVPN for Android"
print N ""

if [ "$1" = "--help" ] || [ "$1" = "" ]; then
  print G "Usage:"
  print N "\t$0 <path to QT>"
  print N ""
  exit 0
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

qmake -v &>/dev/null || die "qmake doesn't exist or it fails"


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

/Users/mozilla/Qt/5.15.1/android/bin/qmake -spec android-clang \
  CONFIG+=debug \
  CONFIG+=qml_debug \
  ANDROID_ABIS="armeabi-v7a" \
  ..//mozillavpn.pro || die "Qmake failed"

print Y "Compiling apk_install_target in /.tmp/"
make -j16 apk_install_target || die "Compile of QT project failed"

print Y "Bundleing APK"
cd src/
make apk || die "Compile of QT project failed"
print G "All done!"
