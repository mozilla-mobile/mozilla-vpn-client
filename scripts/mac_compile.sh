#!/bin/bash

. $(dirname $0)/commons.sh

print N "This script compiles MozillaVPN for MacOS"
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

if ! [ -d "src" ] || ! [ -d "macos" ]; then
  die "This script must be executed at the root of the repository."
fi

if [ "$QTBIN" ]; then
  QMAKE=$QTBIN/qmake
else
  QMAKE=qmake
fi

$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

printn Y "Apply my monotonic patch... "
cp macos/goruntime-boottime-over-monotonic.diff wireguard-apple/wireguard-go-bridge || die "Failed"
print G "done."

print Y "Compile wireguard-go-bridge..."
(cd wireguard-apple/wireguard-go-bridge && ARCHS=x86_64 GOARCH_armv7= make) || die "Compilation failed"
print G "done."

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
ruby scripts/xcode_patcher.rb || die "Failed to merge xcode with wireguard"
print G "done."

print Y "Compiling..."
xcodebuild -project mozillavpn.xcodeproj
