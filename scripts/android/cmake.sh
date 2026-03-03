#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi

cleanup() {
  if [ -f "$WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup" ]; then
    rm -f $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml
    mv $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml
  fi
}

cleanup_and_die() {
  cleanup
  die
}


JOBS=24
RELEASE=1
export SPLITAPK=0
export ARCH="arm64-v8a"
FLAVOR="all"
WORKSPACE_ROOT="$( cd "$(dirname "$0")/../.." ; pwd -P )"

help_function() {
  print G "Usage:"
  print N "\t$0 <path to QT> [-d|--debug] [-j|--jobs <jobs>] [-A | --arch <architectures to build>] [-f | --flavor <flavor>] [--sentrydsn <dsn>] [--sentryendpoint <endpoint>]"
  print N ""
  print N "By default, the android build is compiled in release mode. Use -d or --debug for a debug build."
  print N ""
  print N "Valid architecture values: x86 x86_64 armeabi-v7a arm64-v8a, by default it will use all"
  print N ""
  print N "Valid flavor values: playstore, website, all. By default it will build all flavors."
  print N ""
  exit 0
}
print N "This script compiles MozillaVPN for Android"

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
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
  -f | --flavor)
    FLAVOR="$2"
    shift
    shift
    ;;
  -h | --help)
    help_function
    ;;
  *)
    if [[ "$$1" ]]; then
      QTPATH="$1"
    fi
    shift
    ;;
  esac
done

echo $QTPATH

if ! [[ "$QTPATH" ]]; then
  help_function
fi

printn Y "Mode: "
if [[ "$RELEASE" ]]; then
  print G "release"
else
  print G "debug"
fi

print Y "Flavor: $FLAVOR"
if [[ "$FLAVOR" != "playstore" && "$FLAVOR" != "website" && "$FLAVOR" != "all" ]]; then
  die "Invalid flavor '$FLAVOR'. Valid values are: playstore, website, all"
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


printn Y "Cleaning the folder... "
print G "done."

rm -rf .tmp/src/android-build || die "Failed to remove the temporary directory"
mkdir -p .tmp || die "Failed to create the temporary directory"

# If not provided - use the current time.
if [ -z "$BUILD_TIMESTAMP" ]; then
  BUILD_TIMESTAMP=$(date +%s)
fi

printn Y "Computing the version... "
export SHORTVERSION=$(cat version.txt) # Export so gradle can pick it up
export VERSIONCODE=$(echo "$BUILD_TIMESTAMP" | sed 's/.\{2\}$//' ) #Remove the last 2 digits of the timestamp, should give us a new version every 100 seconds.
FULLVERSION=$SHORTVERSION.$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION - $VERSIONCODE"

build_flavor() {
  local FLAVOR_NAME="$1"
  local BUILD_DIR=".tmp/${FLAVOR_NAME}"

  print Y "Configuring the android build for flavor: $FLAVOR_NAME"
  # Warning: this is hacky.
  #
  # We build the Glean Android SDK from scratch in order to have it linked to the qtglean binary instead of the default glean one.
  # In order to do that we need to generate the Glean internal Kotlin bindings.
  #
  # We need to change the name of the binary in the Uniffi UDL file inside the Glean folder
  # for this to work.
  #
  # Here we go
  if [ ! -f "$WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup" ]; then
    mv $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup
  fi
  cp $WORKSPACE_ROOT/qtglean/uniffi.toml $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml

  local EXTRA_CMAKE_ARGS=""
  if [[ "$FLAVOR_NAME" == "website" ]]; then
    EXTRA_CMAKE_ARGS="-DMZ_ANDROID_WEBSITE_BUILD=ON"
  fi

  if [[ "$RELEASE" ]]; then
    printn Y "Use release config for $FLAVOR_NAME"
    $QTPATH/bin/qt-cmake \
      -DQT_HOST_PATH=$QT_HOST_PATH \
      -DQT_ANDROID_BUILD_ALL_ABIS=TRUE \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT \
      -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTS=OFF \
      $EXTRA_CMAKE_ARGS \
      -GNinja \
      -S . -B "$BUILD_DIR"
  else
    printn Y "Use debug config for $FLAVOR_NAME\n"
    $QTPATH/bin/qt-cmake \
      -DQT_HOST_PATH=$QT_HOST_PATH \
      -DQT_ANDROID_BUILD_ALL_ABIS=FALSE \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT \
      -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_TESTS=OFF \
      $EXTRA_CMAKE_ARGS \
      -GNinja \
      -S . -B "$BUILD_DIR"
  fi

  print Y "Compiling apk_install_target in $BUILD_DIR/"
  # This compiles the client and generates a mozillavpn.so
  cmake --build "$BUILD_DIR" -j$JOBS

  # Restore the original uniffi.toml after the build
  if [ -f "$WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup" ]; then
    rm $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml
    cp $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml.backup $WORKSPACE_ROOT/3rdparty/glean/glean-core/uniffi.toml
  fi

  # Generate a valid gradle project and pre-compile it.
  print Y "Generate Android Project for $FLAVOR_NAME"

  pushd "$BUILD_DIR/src/android-build/" > /dev/null || cleanup_and_die
  # This will combine the qt-libs + qt-resources and the client
  # Into a single gradle project
  if [[ "$RELEASE" ]]; then
    print Y "Generating Release APK for $FLAVOR_NAME..."
    ./gradlew compileReleaseSources || cleanup_and_die
    ./gradlew "assembleRelease" || cleanup_and_die

    print G "Done 🎉"
    print G "Your $FLAVOR_NAME Release APK is under: $BUILD_DIR/src/android-build/build/outputs/apk/$FLAVOR_NAME/release"
  else
    print Y "Generating Debug APK for $FLAVOR_NAME..."
    ./gradlew compileDebugSources || cleanup_and_die
    ./gradlew "assembleDebug" || cleanup_and_die

    print G "Done 🎉"
    print G "Your $FLAVOR_NAME Debug APK is under: $BUILD_DIR/src/android-build/build/outputs/apk/$FLAVOR_NAME/debug"
  fi
  popd > /dev/null
}

# Build the requested flavors
if [[ "$FLAVOR" == "all" ]]; then
  build_flavor "playstore"
  build_flavor "website"
else
  build_flavor "$FLAVOR"
fi

cleanup
