#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
. $(dirname $0)/commons.sh

if [ -f .env ]; then
  . .env
fi

JOBS=8
QTPATH=
RELEASE=1
PROD=

helpFunction() {
  print G "Usage:"
  print N "\t$0 <path to QT> [-d|--debug] [-j|--jobs <jobs>] [-p|--prod]"
  print N ""
  print N "By default, the android build is compiled in release mode. Use -d or --debug for a debug build."
  print N "By default, the project is compiled in staging mode. If you want to use the production env, use -p or --prod."
  print N ""
  exit 0
}

print N "This script compiles MozillaVPN for Android"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -j | --jobs)
    JOBS="$2"
    shift
    shift
    ;;
  -d | --debug)
    RELEASE=
    shift
    ;;
  -p | --prod)
    PROD=1
    shift
    ;;
  -h | --help)
    helpFunction
    ;;
  *)
    if [[ "$QTPATH" ]]; then
      helpFunction
    fi

    QTPATH="$1"
    shift
    ;;
  esac
done


if ! [[ "$QTPATH" ]]; then
  helpFunction
fi

printn Y "Mode: "
if [[ "$RELEASE" ]]; then
  print G "release"
else
  print G "debug"
fi

PRODMODE=
printn Y "Production mode: "
if [[ "$PROD" ]]; then
  print G yes
  PRODMODE="CONFIG+=production"
else
  print G no
fi

if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi

if ! [ -d "$QTPATH/android/bin/" ]; then
  die "QTAndroid SDK was not found in the provided QT path"
fi
print Y "Checking Enviroment"
if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi
if ! [ -d "$QTPATH/android/bin/" ]; then
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

$QTPATH/android/bin/qmake -v &>/dev/null || die "qmake doesn't exist or it fails"


printn Y "Cleaning the folder... "
make distclean &>/dev/null;
print G "done."

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print Y "Importing translation files..."
python3 scripts/importLanguages.py $([[ "$PROD" ]] && echo "-p" || echo "") || die "Failed to import languages"

printn Y "Computing the version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$SHORTVERSION.$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION"

print Y "Configuring the android build"

cd .tmp/

if [[ "$RELEASE" ]]; then
  # On release builds only QT requires these *_metatypes.json
  # The files are actually all the same, but named by _ABI_ (they only differ for plattforms e.g android/ and ios/ )
  # But sometimes the resolver seems to miss the current abi and defaults to the "none" abi
  # This one was missing on my machine, let's create a "none" version in case the resolver might fail too
  printn Y "Patch qt meta data"
  cp $QTPATH/android/lib/metatypes/qt5quick_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5quick_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5charts_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5charts_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5svg_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5svg_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5widgets_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5widgets_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5gui_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5gui_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5qmlmodels_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5qmlmodels_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5qml_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5qml_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5networkauth_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5networkauth_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5network_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5network_xmetatypes.json
  cp $QTPATH/android/lib/metatypes/qt5test_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5test_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5androidextras_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5androidextras_metatypes.json
  cp $QTPATH/android/lib/metatypes/qt5core_armeabi-v7a_metatypes.json $QTPATH/android/lib/metatypes/qt5core_metatypes.json
  printn Y "Use release config"
  $QTPATH/android/bin/qmake -spec android-clang \
    VERSION=$SHORTVERSION \
    CONFIG+=qtquickcompiler \
    CONFIG-=debug \
    CONFIG-=debug_and_release \
    CONFIG+=release \
    $PRODMODE \
    ANDROID_ABIS="armeabi-v7a x86 arm64-v8a" \
    ..//mozillavpn.pro  || die "Qmake failed"
else
  printn Y "Use debug config \n"
  $QTPATH/android/bin/qmake -spec android-clang \
    VERSION=$SHORTVERSION \
    CONFIG+=debug \
    CONFIG-=debug_and_release \
    CONFIG-=release \
    CONFIG+=qml_debug \
    $PRODMODE \
    ANDROID_ABIS="armeabi-v7a" \
    ..//mozillavpn.pro || die "Qmake failed"
fi

print Y "Compiling apk_install_target in .tmp/"
make -j $JOBS sub-src-apk_install_target || die "Compile of QT project failed"

print Y "Bundleing APK"
cd src/
make apk || die "Compile of QT project failed"
print G "All done!"

print N "Your .APK is Located in .tmp/src/android-build/mozillavpn.apk"
