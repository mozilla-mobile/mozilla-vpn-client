#!/bin/bash

. $(dirname $0)/commons.sh

print N "This script compiles Qt5 statically"
print N ""

if [ "$1" = "" ] || [ "$2" = "" ] || [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
  print G "Usage:"
  print N "\t$0 <QT_source_folder> <destination_folder> [anything else will be use as argument for the QT configure script]"
  print N ""
  exit 0
fi

[ -d "$1" ] || die "Unable to find the QT source folder."

cd "$1" || die "Unable to enter into the QT source folder"

shift

PREFIX=$1
shift

printn Y "Cleaning the folder... "
#make distclean -j8 &>/dev/null;
print G "done."

LINUX="
  -platform linux-clang \
  -no-linuxfb \
  -xcb \
"

MACOS="
  -appstore-compliant \
  -no-dbus
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

print Y "Wait..."
./configure \
  $* \
  --prefix=$PREFIX \
  --recheck-all \
  -opensource \
  -confirm-license \
  -release \
  -static \
  -strip \
  -silent \
  -no-compile-examples \
  -nomake tests \
  -nomake tools \
  -no-sql-psql \
  -no-sql-sqlite \
  -skip qt3d \
  -skip webengine \
  -skip qtmultimedia \
  -skip qtserialport \
  -skip qtsensors \
  -skip qtwebsockets \
  -skip qtxmlpatterns \
  -skip qtcharts \
  -skip qtgamepad \
  -skip qtwebchannel \
  -skip qtandroidextras \
  -feature-imageformat_png \
  -qt-libpng \
  -no-feature-cups \
  -no-feature-d3d12 \
  -no-feature-assistant \
  -no-feature-sqlmodel \
  -no-feature-socks5 \
  -no-feature-whatsthis \
  -no-feature-valgrind \
  -no-feature-testlib_selfcover \
  -no-feature-imageformat_jpeg \
  -no-feature-location-labs-plugin \
  -no-feature-geoservices_esri \
  -no-feature-geoservices_here \
  -no-feature-geoservices_itemsoverlay \
  -no-feature-geoservices_mapbox \
  -no-feature-geoservices_mapboxgl \
  -no-feature-geoservices_osm \
  -no-feature-quick-designer \
  -no-feature-distancefieldgenerator \
  -no-feature-kmap2qmap \
  -no-feature-linguist \
  -no-feature-qdbus \
  -no-feature-qtdiag \
  -no-feature-qml-preview \
  -no-feature-printpreviewwidget \
  -no-feature-printpreviewdialog \
  $PLATFORM || die "Configuration error."

print Y "Compiling..."
make -j8 || die "Make failed"

print Y "Installing..."
make -j8 install || die "Make install failed"

print G "All done!"
