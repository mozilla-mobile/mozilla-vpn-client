#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi


JOBS=8
QTPATH=
RELEASE=1
ADJUST_SDK_TOKEN=
export SPLITAPK=0
export ARCH="arm64-v8a"

helpFunction() {
  print G "Usage:"
  print N "\t$0 <path to QT> [-d|--debug] [-j|--jobs <jobs>] [-a|--adjusttoken <adjust_token>]  [-A | --arch <architectures to build>] [--sentrydsn <dsn>] [--sentryendpoint <endpoint>]"
  print N ""
  print N "By default, the android build is compiled in release mode. Use -d or --debug for a debug build."
  print N ""
  print N "If MVPN_ANDROID_ADJUST_TOKEN env is found, this will be used at compilation time."
  print N "Valid architecture values: x86 x86_64 armeabi-v7a arm64-v8a, by default it will use all"
  print N ""
  exit 0
}
print N "This script compiles MozillaVPN for Android"
echo $QT_HOST_PATH

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
  --sentrydsn)
    SENTRY_DSN="$2"
    shift
    shift
    ;;
  --sentryendpoint)
    SENTRY_ENVELOPE_ENDPOINT="$2"
    shift
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


printn Y "Cleaning the folder... "
print G "done."

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print Y "Importing translation files..."
python3 scripts/utils/import_languages.py || die "Failed to import languages"

print Y "Patch Adjust files..."
./scripts/android/patch_adjust.sh

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

# Just in case... This has been a problem before.
chmod +x $QTPATH/bin/qt-cmake

if [[ "$RELEASE" ]]; then
  printn Y "Use release config"
  $QTPATH/bin/qt-cmake \
    -DQT_HOST_PATH=$QT_HOST_PATH \
    -DQT_ANDROID_BUILD_ALL_ABIS=TRUE \
    -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT \
    -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
    -DCMAKE_BUILD_TYPE=Release \
    -DADJUST_TOKEN=$ADJUST_SDK_TOKEN \
    -DSENTRY_DSN=$SENTRY_DSN \
    -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
    -S . -B .tmp/
else
  printn Y "Use debug config \n"
  $QTPATH/bin/qt-cmake \
    -DQT_HOST_PATH=$QT_HOST_PATH \
    -DQT_ANDROID_BUILD_ALL_ABIS=FALSE \
    -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT \
    -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSENTRY_DSN=$SENTRY_DSN \
    -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
    -S . -B .tmp/

fi

print Y "Compiling apk_install_target in .tmp/"
# This compiles the client and generates a mozillavpn.so
cmake --build .tmp -j$JOBS

# Generate a valid gradle project and pre-compile it.
print Y "Generate Android Project"

#androiddeployqt --input .tmp/src/android-mozillavpn-deployment-settings.json --output .tmp/src/android-build || die


cd .tmp/src/android-build/
# This will combine the qt-libs + qt-resources and the client
# Into a single gradle project
if [[ "$RELEASE" ]]; then
  print Y "Generating Release APK..."
  ./gradlew compileReleaseSources
  ./gradlew assemble || die

  print G "Done 🎉"
  print G "Your Release APK is under .tmp/src/android-build/build/outputs/apk/release/"
else
  print Y "Generating Debug APK..."
  ./gradlew compileDebugSources
  ./gradlew assembleDebug || die
  print G "Done 🎉"
  print G "Your Debug APK is under .tmp/src/android-build/build/outputs/apk/debug/"
fi
