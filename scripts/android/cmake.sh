#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi

cleanup() {
  rm $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml
  mv $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml
}

cleanup_and_die() {
  cleanup
  die
}


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
  -h | --help)
    helpFunction
    ;;
  *)
    if [[ "$$1" ]]; then
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

if ! [ -d "$QTPATH/bin" ]; then
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

# If not provided - use the current time.
if [ -z "$BUILD_TIMESTAMP" ]; then
  BUILD_TIMESTAMP=$(date +s)
fi

printn Y "Computing the version... "
export SHORTVERSION=$(cat version.txt) # Export so gradle can pick it up
export VERSIONCODE=$(echo "$BUILD_TIMESTAMP" | sed 's/.\{2\}$//' ) #Remove the last 2 digits of the timestamp, should give us a new version every 100 seconds.
export ADJUST_SDK_TOKEN=$ADJUST_SDK_TOKEN # Export it even if it is not set to override system env variables
FULLVERSION=$SHORTVERSION.$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION - $VERSIONCODE"
print Y "Configuring the android build"

# Warning: this is hacky.
#
# We build the Glean Android SDK from scratch in order to have it linked to the qtglean binary instead of the default glean one.
# In order to do that we need to generate the Glean internal Kotlin bindings.
#
# We need to change the name of the binary in the Uniffi UDL file inside the Glean folder
# for this to work.
#
# Here we go
mv $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup
cp $WORKSPACE_ROOT/qtglean/uniffi.toml 3rdparty/glean/glean-core/uniffi.toml

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
    -DBUILD_TESTS=OFF \
    -GNinja \
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
    -DBUILD_TESTS=OFF \
    -GNinja \
    -S . -B .tmp/

fi

print Y "Compiling apk_install_target in .tmp/"
# This compiles the client and generates a mozillavpn.so
cmake --build .tmp -j$JOBS

# Generate a valid gradle project and pre-compile it.
print Y "Generate Android Project"

#androiddeployqt --input .tmp/src/android-mozillavpn-deployment-settings.json --output .tmp/src/android-build || cleanup_and_die

cd .tmp/src/android-build/
# This will combine the qt-libs + qt-resources and the client
# Into a single gradle project
if [[ "$RELEASE" ]]; then
  print Y "Generating Release APK..."
  ./gradlew compileReleaseSources
  ./gradlew assemble -Padjusttoken=$ADJUST_SDK_TOKEN || cleanup_and_die

  print G "Done 🎉"
  print G "Your Release APK is under .tmp/src/android-build/build/outputs/apk/release/"
else
  print Y "Generating Debug APK..."
  ./gradlew compileDebugSources
  ./gradlew assembleDebug || cleanup_and_die
  print G "Done 🎉"
  print G "Your Debug APK is under .tmp/src/android-build/build/outputs/apk/debug/"
fi

cleanup
