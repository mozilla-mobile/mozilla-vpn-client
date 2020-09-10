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

printn Y "Cleaning the existing project... "
rm -rf mozillavpn.xcodeproj/ || die "Failed to remove things"
print G "done."

print Y "Creating the xcode project via qmake..."
$QMAKE \
  QTPLUGIN+=qsvg \
  CONFIG-=static \
  MACOS_INTEGRATION=1 \
  -spec macx-xcode \
  src/src.pro  || die "Compilation failed"

print Y "Patching the xcode project..."
ruby scripts/xcode_patcher.rb
print G "done."

print Y "Compiling..."
xcodebuild -project mozillavpn.xcodeproj
