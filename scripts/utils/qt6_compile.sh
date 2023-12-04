#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

POSITIONAL=()
JOBS=8

helpFunction() {
  print G "Usage:"
  print N "\t$0 <QT_source_folder> <destination_folder> [-j|--jobs <jobs>] [anything else will be use as argument for the QT configure script]"
  print N ""
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

cd "$1" || die "Unable to enter into the QT source folder"

shift

PREFIX=$1
shift

printn Y "Cleaning the folder... "
make distclean -j $JOBS &>/dev/null;
print G "done."

LINUX="
  -platform linux-clang \
  -egl \
  -opengl es2 \
  -no-icu \
  -no-linuxfb \
  -bundled-xcb-xinput \
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

  # There is a QT-Linguist GUI tool that 
  # we cannot feature flag off, 
  # also it does not build on macos ... so nuklear option here.
  rm -rf qttools/src/linguist/linguist
  mkdir qttools/src/linguist/linguist
  echo "return()" > qttools/src/linguist/linguist/CMakeLists.txt
  PLATFORM=$MACOS
else
  die "Unsupported platform (yet?)"
fi

print Y "Wait..."
bash ./configure \
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
  -no-feature-qdbus \
  -no-feature-qtattributionsscanner \
  -no-feature-qtdiag \
  -no-feature-qtplugininfo \
  -no-feature-pixeltool \
  -no-feature-distancefieldgenerator \
  -no-feature-designer \
  -no-feature-assistant \
  -no-feature-qml-xml-http-request \
  -no-feature-qml-network \
  -no-feature-tiff \
  -no-feature-webp \
  -no-feature-cups \
  -no-feature-style-fusion \
  -no-feature-style-mac \
  -no-feature-style-windows \
  -no-feature-textmarkdownwriter \
  -no-feature-cssparser \
  -no-feature-qmake \
  -no-feature-itemmodeltester \
  -no-feature-quick-sprite \
  -no-feature-quick-tableview \
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
  $PLATFORM || die "Configuration error."

print Y "Compiling..."
cmake --build . --parallel $JOBS || die "Make failed"

print Y "Installing..."
cmake --install . || die "Make install failed"

print G "All done!"
