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

WINDOWS="
  -platform win32-msvc \
  -no-feature-dynamicgl \
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
  -no-feature-style-fusion \
  -no-feature-style-windows \
  -no-feature-style-windowsvista \
  -no-feature-quickcontrols2-material \
  -no-feature-quickcontrols2-macos \
  -no-feature-quickcontrols2-ios \
  -no-feature-sql-sqlite \
  -no-feature-sql \
  -no-feature-textodfwriter \
  -no-feature-networklistmanager \
  -no-feature-dbus \
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
  -skip qtwebchannel  \
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
  -skip qtwayland  \
  -skip qtvirtualkeyboard  \
  -skip qtwebview  \
   -- \

"


MACOS="
  -appstore-compliant \
  -no-feature-qdbus \
  -no-dbus \
  -- \
  -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'
"
if [[ "$CMAKE_SYSTEM_NAME" == "Windows"* ]]; then
# We're cross compiling for windows!
  print N "Configure for windows"
  PLATFORM=$WINDOWS
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
  print N "Configure for linux"
  PLATFORM=$LINUX
elif [[ "$OSTYPE" == "darwin"* ]]; then
  print N "Configure for darwin"
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
  -nomake tests \
  -make libs \
  -skip qt3d \
  -skip qtwebengine \
  -skip qtmultimedia \
  -skip qtserialport \
  -skip qtsensors \
  -skip qtgamepad \
  -skip qtwebchannel \
  -skip qtandroidextras \
  -feature-imageformat_png \
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
