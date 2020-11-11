#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

print N "This script compiles MozillaVPN for MacOS/iOS"
print N ""

if [ "$1" != "macos" ] && [ "$1" != "ios" ]; then
  print G "Usage:"
  print N "\t$0 <macos|ios>"
  print N ""
  print G "Config variables:"
  print N "\tQT_MACOS_BIN=</path/of/the/qt/bin/folder/for/macos>"
  print N "\tQT_IOS_BIN=</path/of/the/qt/bin/folder/for/ios>"
  print N ""
  exit 0
fi

if ! [ -d "src" ] || ! [ -d "ios" ] || ! [ -d "macos" ]; then
  die "This script must be executed at the root of the repository."
fi

QMAKE=qmake
if [ "$1" = "macos" ] && ! [ "$QT_MACOS_BIN" = "" ]; then
  QMAKE=$QT_MACOS_BIN/qmake
  PATH=$QT_MACOS_BIN:$PATH
elif [ "$1" = "ios" ] && ! [ "$QT_IOS_BIN" = "" ]; then
  QMAKE=$QT_IOS_BIN/qmake
  PATH=$QT_IOS_BIN:$PATH
fi

$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

printn Y "Retrieve the wireguard-go version... "
(cd 3rdparty/wireguard-apple/wireguard-go-bridge && go list -m golang.zx2c4.com/wireguard | sed -n 's/.*v\([0-9.]*\).*/#define WIREGUARD_GO_VERSION "\1"/p') > macos/gobridge/wireguard-go-version.h
print G "done."

printn Y "Apply my monotonic patch... "
cp macos/gobridge/goruntime-boottime-over-monotonic.diff 3rdparty/wireguard-apple/wireguard-go-bridge || die "Failed"
print G "done."

printn Y "Cleaning the existing project... "
rm -rf mozillavpn.xcodeproj/ || die "Failed to remove things"
print G "done."

print Y "Importing translation files..."
python3 scripts/importLanguages.py || die "Failed to import"

printn Y "Extract the project version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$SHORTVERSION.$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION"

MACOS_FLAGS="
  QTPLUGIN+=qsvg
  CONFIG-=static
  MACOS_INTEGRATION=1
"

IOS_FLAGS="
  IOS_INTEGRATION=1
"

if [ "$1" = "macos" ]; then
  PLATFORM=$MACOS_FLAGS
elif [ "$1" = "ios" ]; then
  PLATFORM=$IOS_FLAGS
else
  die "Why we are here?"
fi

print Y "Creating the xcode project via qmake..."
$QMAKE \
  VERSION=$FULLVERSION \
  CONFIG-=debug \
  CONFIG+=release \
  -spec macx-xcode \
  CONFIG-=debug \
  CONFIG+=release \
  $PLATFORM \
  src/src.pro || die "Compilation failed"

print Y "Patching the xcode project..."
ruby scripts/xcode_patcher.rb "MozillaVPN.xcodeproj" "$SHORTVERSION" "$FULLVERSION" "$1" || die "Failed to merge xcode with wireguard"
print G "done."

print Y "Opening in XCode..."
open MozillaVPN.xcodeproj
print G "All done!"
