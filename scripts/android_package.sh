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
ADJUST_SDK_TOKEN=
export SPLITAPK=0
# export ARCH="x86 x86_64 armeabi-v7a arm64-v8a"

helpFunction() {
  print G "Usage:"
  print N "\t$0 <path to QT> [-d|--debug] [-j|--jobs <jobs>] [-a|--adjusttoken <adjust_token>]  [-A | --arch] architectures to build"
  print N ""
  print N "By default, the android build is compiled in release mode. Use -d or --debug for a debug build."
  print N ""
  print N "If MVPN_ANDROID_ADJUST_TOKEN env is found, this will be used at compilation time."
  print N "Valid architecture values: x86 x86_64 armeabi-v7a arm64-v8a, by default it will use all"
  print N ""
  exit 0
}

print N "This script compiles MozillaVPN for Android"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -a | --adjusttoken)
    ADJUST_SDK_TOKEN="$2"
    shift
    shift
    ;;
  -A | --arch)
    ARCH="$2"
    shift
    shift
    ;;
  -j | --jobs)
    JOBS="$2"
    shift
    shift
    ;;
  -d | --debug)
    RELEASE=
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

if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi

if [ -d "$QTPATH/android/bin/" ]; then
  QTPATH=$QTPATH/android
elif ! [ -d "$QTPATH/bin/" ]; then
  die "QTAndroid SDK was not found in the provided QT path"
fi

print Y "Checking Enviroment"
if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi
if [ -z "${ANDROID_NDK_ROOT}" ]; then
  die "Could not find 'ANDROID_NDK_ROOT' in env"
fi
if [ -z "${ANDROID_SDK_ROOT}" ]; then
  die "Could not find 'ANDROID_SDK_ROOT' in env"
fi

if ! [[ "$ADJUST_SDK_TOKEN" ]] && [[ "$MVPN_ANDROID_ADJUST_TOKEN" ]]; then
  print Y "Using the MVPN_ANDROID_ADJUST_TOKEN value for the adjust token"
  ADJUST_SDK_TOKEN=$MVPN_ANDROID_ADJUST_TOKEN
fi

$QTPATH/bin/qmake -v &>/dev/null || die "qmake doesn't exist or it fails"

printn Y "Cleaning the folder... "
make distclean &>/dev/null;
print G "done."

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print Y "Importing translation files..."
python3 scripts/importLanguages.py || die "Failed to import languages"

print Y "Generating glean samples..."
python3 scripts/generate_glean.py || die "Failed to generate glean samples"

print Y "Copy and patch Adjust SDK..."
rm -rf "android/src/com/adjust" || die "Failed to remove the adjust folder"
cp -a "3rdparty/adjust-android-sdk/Adjust/sdk-core/src/main/java/com/." "android/src/com/" || die "Failed to copy the adjust codebase"
git apply --directory="android/src/" "3rdparty/adjust_https_to_http.diff" || die "Failed to apply the adjust http patch"

printn Y "Computing the version... "
export SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ ) # Export so gradle can pick it up
export VERSIONCODE=$(date +%s | sed 's/.\{3\}$//' )"0" #Remove the last 3 digits of the timestamp, so we only get every ~16m a new versioncode
export ADJUST_SDK_TOKEN=$ADJUST_SDK_TOKEN # Export it even if it is not set to override system env variables
FULLVERSION=$SHORTVERSION.$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION - $VERSIONCODE"
print Y "Configuring the android build"
if [[ "$ADJUST_SDK_TOKEN" ]]; then
  print Y "Use adjust config"
  ADJUST="CONFIG+=adjust"
else
  ADJUST="CONFIG-=adjust"
fi

cd .tmp/

if [[ "$RELEASE" ]]; then
  # On release builds only QT requires these *_metatypes.json
  # The files are actually all the same, but named by _ABI_ (they only differ for plattforms e.g android/ and ios/ )
  # But sometimes the resolver seems to miss the current abi and defaults to the "none" abi
  # This one was missing on my machine, let's create a "none" version in case the resolver might fail too
  printn Y "Patch qt meta data"
  cp $QTPATH/lib/metatypes/qt5quick_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5quick_metatypes.json
  cp $QTPATH/lib/metatypes/qt5charts_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5charts_metatypes.json
  cp $QTPATH/lib/metatypes/qt5svg_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5svg_metatypes.json
  cp $QTPATH/lib/metatypes/qt5widgets_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5widgets_metatypes.json
  cp $QTPATH/lib/metatypes/qt5gui_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5gui_metatypes.json
  cp $QTPATH/lib/metatypes/qt5qmlmodels_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5qmlmodels_metatypes.json
  cp $QTPATH/lib/metatypes/qt5qml_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5qml_metatypes.json
  cp $QTPATH/lib/metatypes/qt5networkauth_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5networkauth_metatypes.json
  cp $QTPATH/lib/metatypes/qt5network_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5network_xmetatypes.json
  cp $QTPATH/lib/metatypes/qt5test_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5test_metatypes.json
  cp $QTPATH/lib/metatypes/qt5androidextras_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5androidextras_metatypes.json
  cp $QTPATH/lib/metatypes/qt5core_armeabi-v7a_metatypes.json $QTPATH/lib/metatypes/qt5core_metatypes.json
  printn Y "Use release config"
  $QTPATH/bin/qmake -spec android-clang \
    VERSION=$SHORTVERSION \
    BUILD_ID=$VERSIONCODE \
    CONFIG+=qtquickcompiler \
    CONFIG-=debug \
    CONFIG-=debug_and_release \
    CONFIG+=release \
    ANDROID_ABIS=$ARCH \
    $ADJUST \
    ..//mozillavpn.pro  || die "Qmake failed"
else
  printn Y "Use debug config \n"
  $QTPATH/bin/qmake -spec android-clang \
    VERSION=$SHORTVERSION \
    BUILD_ID=$VERSIONCODE \
    CONFIG+=debug \
    CONFIG-=debug_and_release \
    CONFIG-=release \
    CONFIG+=qml_debug \
    ANDROID_ABIS=$ARCH \
    $ADJUST \
    ..//mozillavpn.pro || die "Qmake failed"
fi

print Y "Compiling apk_install_target in .tmp/"
make -j $JOBS sub-src-apk_install_target || die "Compile of QT project failed"

# We need to run the debug bundle step in any case
# as this is the only make target that generates the gradle 
# project, that we can then use to generate a "real" release build
print Y "Bundleing (debug) APK"
cd src/
make apk || die "Compile of QT project failed"
print G "All done!"
print N "Your debug .APK is Located in .tmp/src/android-build/mozillavpn.apk"

# If we wanted a release build we now need to 
# also compile the java/kotlin code in release mode
if [[ "$RELEASE" ]]; then
  print Y "Generating Release APK..."
  export SPLITAPK=1
  cd android-build
  ./gradlew compileReleaseSources
  ./gradlew assemble

  print G "Done 🎉"
  print G "Your Release APK is under .tmp/src/android-build/build/outputs/apk/release/android-build-universal-release-unsigned.apk"
fi
