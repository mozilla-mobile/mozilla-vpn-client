#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

POSITIONAL=()
JOBS=8
BUILDDIR=

helpFunction() {
  print G "Usage:"
  print N "\t$0 <QT_source_folder> <destination_folder> [options]"
  print N ""
  print N "Build options:"
  print N "  -j, --jobs NUM   Parallelize build across NUM processes. (default: 8)"
  print N "  -b, --build DIR  Build in DIR. (default: <QT_source_folder>/build)"
  print N "  -h, --help       Display this message and exit."
  print N ""
  print N "Any other arguments will be passed to the Qt configure script."
  exit 0
}

print N "This script compiles Qt6 statically"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
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
  -egl \
  -opengl es2 \
  -no-icu \
  -no-linuxfb \
  -bundled-xcb-xinput \
  -feature-qdbus \
  -xcb \
"

MACOS="
  -skip qtwayland  \
  -no-feature-quickcontrols2-ios \
  -no-feature-quickcontrols2-macos \
  -no-feature-qdbus \
  -appstore-compliant \
  -feature-texthtmlparser \ 
  -- \
  -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'
"

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  print N "Configure for linux"
  PLATFORM=$LINUX
elif [[ "$OSTYPE" == "darwin"* ]]; then
  print N "Configure for darwin"

  PLATFORM=$MACOS
else
  die "Unsupported platform (yet?)"
fi

# There is a QT-Linguist GUI tool that 
# we cannot feature flag off, and itself does not properly 
# check during configure if it can be built. 
# so nuclear option here.
rm -rf qttools/src/linguist/linguist
mkdir qttools/src/linguist/linguist
echo "return()" > qttools/src/linguist/linguist/CMakeLists.txt

# Create the installation prefix, and convert to an absolute path.
mkdir -p $PREFIX
PREFIX=(cd $PREFIX && pwd)

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
  -no-feature-pixeltool \
  -no-feature-qtattributionsscanner \
  -no-feature-qtdiag \
  -no-feature-qtplugininfo \
  -no-feature-pixeltool \
  -no-feature-distancefieldgenerator \
  -no-feature-designer \
  -no-feature-assistant \
  -no-feature-qml-xml-http-request \
  -no-feature-tiff \
  -no-feature-webp \
  -no-feature-cups \
  -no-feature-style-fusion \
  -no-feature-style-mac \
  -no-feature-style-windows \
  -no-feature-textmarkdownwriter \
  -no-feature-cssparser \
  -no-feature-itemmodeltester \
  -no-feature-sql-sqlite \
  -no-feature-sql \
  -no-feature-dbus \
  -no-feature-xml \
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
  -qt-freetype \
  $PLATFORM) || die "Configuration error."

print Y "Compiling..."
cmake --build $BUILDDIR --parallel $JOBS || die "Make failed"

print Y "Installing..."
cmake --install $BUILDDIR || die "Make install failed"

print G "All done!"
