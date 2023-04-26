#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi


JOBS=24
RELEASE=1
ADJUST_SDK_TOKEN=
export SPLITAPK=0
export ARCH="arm64-v8a"
WORKSPACE_ROOT="$( cd "$(dirname "$0")/../.." ; pwd -P )"

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
echo $QTPATH

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
    if ! [[ "$$1" ]]; then
      QTPATH="$1"
    fi
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

rm -rf .tmp/src/android-build || die "Failed to remove the temporary directory"
mkdir -p .tmp || die "Failed to create the temporary directory"

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

if [[ "$RELEASE" ]]; then
  printn Y "Use release config"
  $QTPATH/bin/qt-cmake \
    -DQT_HOST_PATH=$QT_HOST_PATH \
    -DQT_ANDROID_BUILD_ALL_ABIS=TRUE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT \
    -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
    -DCMAKE_BUILD_TYPE=Release \
    -DADJUST_TOKEN=$ADJUST_SDK_TOKEN \
    -DSENTRY_DSN=$SENTRY_DSN \
    -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
    -DCMAKE_AUTOGEN_PARALLEL=1 \
    -S . -B .tmp/
else
  printn Y "Use debug config \n"
  $QTPATH/bin/qt-cmake \
    -DQT_HOST_PATH=$QT_HOST_PATH \
    -DQT_ANDROID_BUILD_ALL_ABIS=FALSE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT \
    -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSENTRY_DSN=$SENTRY_DSN \
    -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
    -DCMAKE_AUTOGEN_PARALLEL=1 \
    -S . -B .tmp/

fi

print Y "Compiling apk_install_target in .tmp/"
# This compiles the client and generates a mozillavpn.so
# Do not Build the default Project, as that will build other targets than vpn
# Also try to build apk's which will fail because we need to prep some glean stuff. 
# See Below. 
cmake --build .tmp -j$JOBS --target mozillavpn_prepare_apk_dir 

# Generate a valid gradle project and pre-compile it.
print Y "Generate Android Project"
# We need to pass --no-build here, otherwise we get one useless timecostly invokation of gradle.
androiddeployqt --input .tmp/src/android-mozillavpn-deployment-settings.json --output .tmp/src/android-build

# Warning: this is hacky.
#
# We build the Glean Android SDK from scratch in order to have it linked to the qtglean binary instead of the default glean one.
# In order to do that we need to generate the Glean internal Kotlin bindings.
#
# We need to change the name of the binary in the Uniffi UDL file inside the Glean folder
# for this to work.
#
# We will change the name of that here and then undo the change after running gradle.
# The changed file has already been generated by a step in cmake, so we will just copy the changed file on top of the glean one.
#
# Here we go
mv $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup
mv $WORKSPACE_ROOT/qtglean/uniffi.toml 3rdparty/glean/glean-core/uniffi.toml

cd .tmp/src/android-build/
# This will combine the qt-libs + qt-resources and the client
# Into a single gradle project
if [[ "$RELEASE" ]]; then
  print Y "Generating Release APK..."
  ./gradlew compileReleaseSources
  ./gradlew assemble

  print G "Done 🎉"
  print G "Your Release APK is under .tmp/src/android-build/build/outputs/apk/release/"

  if [ -n "$IS_DEV_CONTAINER" ]; then
    echo "In dev Container, Copying binary to ./release"
    cp -r build/outputs/apk/release $WORKSPACE_ROOT
  else 
    print G "Your Release APK is under .tmp/src/android-build/build/outputs/apk/release/"
  fi
else
  print Y "Generating Debug APK..."
  ./gradlew compileDebugSources
  ./gradlew assembleDebug
  print G "Done 🎉"

  if [ -n "$IS_DEV_CONTAINER" ]; then
    echo "In dev Container, Copying binary: to ./debug"
    cp -r build/outputs/apk/debug $WORKSPACE_ROOT
  else 
     print G "Your Debug APK is under .tmp/src/android-build/build/outputs/apk/debug/"
  fi
fi

rm $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml
mv $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml
