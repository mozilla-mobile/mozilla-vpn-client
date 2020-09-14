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

printn Y "Retrieve the wireguard-go version... "
(cd wireguard-apple/wireguard-go-bridge && go list -m golang.zx2c4.com/wireguard | sed -n 's/.*v\([0-9.]*\).*/#define WIREGUARD_GO_VERSION "\1"/p') > macos/wireguard-go-version.h
print G "done."

printn Y "Apply my monotonic patch... "
cp macos/goruntime-boottime-over-monotonic.diff wireguard-apple/wireguard-go-bridge || die "Failed"
print G "done."

print Y "Compile wireguard-go-bridge..."
(cd wireguard-apple/wireguard-go-bridge && SDK_NAME=macosx ARCHS=x86_64 GOARCH_armv7= make) || die "Compilation failed"
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

printn Y "Extract the project version..."
VERSION=$(cat src/src.pro  | grep ^VERSION | cut -d= -f2 | tr -d ' ')
print G "$VERSION"

print Y "Patching the xcode project..."
ruby scripts/xcode_patcher.rb "$VERSION" || die "Failed to merge xcode with wireguard"
print G "done."

print Y "Compiling..."
xcodebuild -project mozillavpn.xcodeproj
