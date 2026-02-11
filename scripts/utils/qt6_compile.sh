#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

POSITIONAL=()
JOBS=8
BUILDDIR=
PLATFORM_ARG=
QT_HOST_PATH=
ANDROID_SDK="$HOME/Android/Sdk"
ANDROID_NDK="$HOME/Android/Sdk/ndk/27.2.12479018"

helpFunction() {
  print G "Usage:"
  print N "\t$0 <QT_source_folder> <destination_folder> [options]"
  print N ""
  print N "Build options:"
  print N "  -p, --platform PLATFORM      Target platform: linux, macos, or android. (default: auto-detect)"
  print N "  -j, --jobs NUM               Parallelize build across NUM processes. (default: 8)"
  print N "  -b, --build DIR              Build in DIR. (default: <QT_source_folder>/build)"
  print N "  -h, --help                   Display this message and exit."
  print N ""
  print N "Android-specific options:"
  print N "  --qt-host-path PATH          Path to Qt host tools (required for android)"
  print N "  --android-sdk PATH           Path to Android SDK. (default: ~/Android/Sdk)"
  print N "  --android-ndk PATH           Path to Android NDK. (default: ~/Android/Sdk/ndk/27.2.12479018)"
  print N ""
  print N "Any other arguments will be passed to the Qt configure script."
  exit 0
}

print N "This script compiles Qt6 statically"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -p | --platform)
    PLATFORM_ARG="$2"
    shift
    shift
    ;;
  -j | --jobs)
    JOBS="$2"
    shift
    shift
    ;;
  -b | --build)
    BUILDDIR="$2"
    shift
    shift
    ;;
  --qt-host-path)
    QT_HOST_PATH="$2"
    shift
    shift
    ;;
  --android-sdk)
    ANDROID_SDK="$2"
    shift
    shift
    ;;
  --android-ndk)
    ANDROID_NDK="$2"
    shift
    shift
    ;;
  -h | --help)
    helpFunction
    ;;
  *)
    POSITIONAL+=("$1")
    shift
    ;;
  esac
done

set -- "${POSITIONAL[@]}" # restore positional parameters

if [[ $# -lt 2 ]]; then
  helpFunction
fi

[ -d "$1" ] || die "Unable to find the QT source folder."
SRCDIR=$(cd $1 && pwd)
shift

PREFIX=$1
shift

if [[ -z "$BUILDDIR" ]]; then
  BUILDDIR=$SRCDIR/build
fi

LINUX="
  -platform linux-clang \
  -openssl-linked \
  -egl \
  -opengl es2 \
  -no-icu \
  -no-linuxfb \
  -system-freetype \
  -fontconfig \
  -bundled-xcb-xinput \
  -feature-qdbus \
  -feature-wayland \
  -xcb \
  -- \
  -DOPENSSL_USE_STATIC_LIBS=ON \
"

MACOS="
  -skip qtwayland  \
  -no-feature-qdbus \
  -qt-freetype \
  -appstore-compliant \
  -feature-texthtmlparser \
  -- \
  -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'
"

ANDROID="
  -qt-host-path $QT_HOST_PATH \
  -android-abis arm64-v8a,armeabi-v7a,x86,x86_64 \
  -android-sdk $ANDROID_SDK \
  -android-ndk $ANDROID_NDK
"

# Determine target platform
if [[ -n "$PLATFORM_ARG" ]]; then
  # Platform explicitly specified via argument
  case "$PLATFORM_ARG" in
  linux)
    print N "Configure for linux"
    PLATFORM=$LINUX
    ;;
  macos)
    print N "Configure for macos"
    PLATFORM=$MACOS
    ;;
  android)
    print N "Configure for android"
    # Validate required Android arguments
    if [[ -z "$QT_HOST_PATH" ]]; then
      die "Android build requires --qt-host-path argument"
    fi
    if [[ ! -d "$QT_HOST_PATH" ]]; then
      die "Qt host path does not exist: $QT_HOST_PATH"
    fi
    if [[ ! -d "$ANDROID_SDK" ]]; then
      die "Android SDK path does not exist: $ANDROID_SDK"
    fi
    if [[ ! -d "$ANDROID_NDK" ]]; then
      die "Android NDK path does not exist: $ANDROID_NDK"
    fi
    PLATFORM=$ANDROID
    ;;
  *)
    die "Unsupported platform: $PLATFORM_ARG (supported: linux, macos, android)"
    ;;
  esac
else
  # Auto-detect platform from OSTYPE
  if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    print N "Configure for linux (auto-detected)"
    PLATFORM=$LINUX
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    print N "Configure for macos (auto-detected)"
    PLATFORM=$MACOS
  else
    die "Unsupported platform (yet?)"
  fi
fi

# There is a QT-Linguist GUI tool that 
# we cannot feature flag off, and itself does not properly 
# check during configure if it can be built. 
# so nuclear option here.
# rm -rf qttools/src/linguist/linguist
# mkdir qttools/src/linguist/linguist
# echo "return()" > qttools/src/linguist/linguist/CMakeLists.txt

# Create the installation prefix, and convert to an absolute path.
mkdir -p $PREFIX
PREFIX=$(cd $PREFIX && pwd)

print Y "Wait..."
mkdir -p $BUILDDIR
(cd $BUILDDIR && bash $SRCDIR/configure \
  $* \
  --prefix=$PREFIX \
  -opensource \
  -confirm-license \
  -release \
  -static \
  -strip \
  -silent \
  -nomake tests \
  -make libs \
  -no-feature-sql-odbc \
  -no-feature-qtattributionsscanner \
  -no-feature-qtdiag \
  -no-feature-qtplugininfo \
  -no-feature-pixeltool \
  -no-feature-distancefieldgenerator \
  -no-feature-assistant \
  -no-feature-tiff \
  -no-feature-webp \
  -no-feature-cups \
  -no-feature-textmarkdownwriter \
  -no-feature-itemmodeltester \
  -no-feature-sql-sqlite \
  -no-feature-sql \
  -skip qt3d  \
  -skip qtdoc \
  -skip qtgrpc \
  -skip qtconnectivity \
  -skip qtquickeffectmaker \
  -skip qtquicktimeline \
  -skip qtwebengine  \
  -skip qtwebview \
  -skip qtlocation \
  -skip qtmultimedia  \
  -skip qtserialport  \
  -skip qtsensors  \
  -skip qtgamepad  \
  -skip qtgraphs \
  -skip qtandroidextras  \
  -skip qtquick3dphysics \
  -skip qtactiveqt  \
  -skip qtcharts  \
  -skip qtcoap  \
  -skip qtdatavis3d  \
  -skip qtgrpc  \
  -skip qtremoteobjects  \
  -skip qtlottie  \
  -skip qtmqtt  \
  -skip qtopcua  \
  -skip qtpositioning  \
  -skip qtquick3d  \
  -skip qtscxml  \
  -skip qtserialbus  \
  -skip qtserialport  \
  -skip qtspeech  \
  -skip qtvirtualkeyboard  \
  -skip qtweb \
  -feature-imageformat_png \
  -feature-optimize_full \
  -feature-xml \
  -qt-doubleconversion \
  -qt-libpng \
  -qt-zlib \
  -qt-pcre \
  $PLATFORM) || die "Configuration error."

print Y "Compiling..."
cmake --build $BUILDDIR --parallel $JOBS || die "Make failed"

print Y "Installing..."
cmake --install $BUILDDIR || die "Make install failed"

print G "All done!"
