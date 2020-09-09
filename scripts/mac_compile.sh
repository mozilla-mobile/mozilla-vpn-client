#!/bin/bash

. $(dirname $0)/commons.sh

print N "This script compiles MozillaVPN for linux using a static Qt5 build"
print N ""

if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
  print G "Usage:"
  print N "\t$0"
  print N ""
  print G "Config variables:"
  print N "\tQTBIN=</path/of/the/qt/bin/folder>"
  print N ""
  exit 0
fi

if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi

if [ "$QTBIN" ]; then
  QMAKE=$QTBIN/qmake
else
  QMAKE=qmake
fi

$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

printn Y "Cleaning the folder... "
make distclean &>/dev/null;
print G "done."

print Y "Configuring the build (qmake)..."
$QMAKE \
  CONFIG+=static \
  QTPLUGIN+=qsvg \
  PREFIX=$1 || die "Compilation failed"

print Y "Compiling..."
make -j8
print G "Compilation completed!"

print Y "Installation..."
make install
print G "Installation completed!"
