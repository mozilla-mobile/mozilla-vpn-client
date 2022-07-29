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

if ! [ -d "$QTPATH/bin/" ]; then
  die "QTAndroid SDK was not found in the provided QT path"
fi

QT_HOST=$($QTPATH/bin/qmake -query QT_HOST_PREFIX)
QT_ROOT=$QTPATH/../


if ! [[ "$ADJUST_SDK_TOKEN" ]] && [[ "$MVPN_ANDROID_ADJUST_TOKEN" ]]; then
  print Y "Using the MVPN_ANDROID_ADJUST_TOKEN value for the adjust token"
  ADJUST_SDK_TOKEN=$MVPN_ANDROID_ADJUST_TOKEN
fi


print Y "Importing translation files..."
echo "$QT_HOST"
python3 scripts/utils/import_languages.py -q $QT_HOST|| die "Failed to import languages"

print Y "Generating glean samples..."
python3 scripts/utils/generate_glean.py -j "android/src/" || die "Failed to generate glean samples"

./scripts/android/patch_adjust.sh 
GRADLE_ARGS="-p android  -Pmozillavpn_qt_host_dir=$QT_HOST -Pmozillavpn_qt_android_dir=$QT_ROOT"

if [[ "$RELEASE" ]]; then
  print Y "Generating Release APK..."
  android/gradlew compileReleaseSources $GRADLE_ARGS
  android/gradlew assemble $GRADLE_ARGS || die

  print G "Done 🎉"
  print G "Your Release APK is under .tmp/src/android-build/build/outputs/apk/release/"
else
  print Y "Generating Debug APK..."
  android/gradlew compileDebugSources $GRADLE_ARGS
  android/gradlew assembleDebug  $GRADLE_ARGS || die
  print G "Done 🎉"
  print G "Your Debug APK is under .tmp/src/android-build/build/outputs/apk/debug/"
fi
