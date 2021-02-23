#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

if [ -f .env ]; then
  . .env
fi

RELEASE=1
OS=
PROD=
NETWORKEXTENSION=

helpFunction() {
  print G "Usage:"
  print N "\t$0 <macos|ios> [-d|--debug] [-p|--prod] [-n|--networkextension]"
  print N ""
  print N "By default, the project is compiled in release mode. Use -d or --debug for a debug build."
  print N "By default, the project is compiled in staging mode. If you want to use the production env, use -p or --prod."
  print N "Use -n or --networkextension to force the network-extension component for MacOS too."
  print N ""
  print G "Config variables:"
  print N "\tQT_MACOS_BIN=</path/of/the/qt/bin/folder/for/macos>"
  print N "\tQT_IOS_BIN=</path/of/the/qt/bin/folder/for/ios>"
  print N ""
  exit 0
}

print N "This script compiles MozillaVPN for MacOS/iOS"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -d | --debug)
    RELEASE=
    shift
    ;;
  -p | --prod)
    PROD=1
    shift
    ;;
  -n | --networkextension)
    NETWORKEXTENSION=1
    shift
    ;;
  -h | --help)
    helpFunction
    ;;
  *)
    if [[ "$OS" ]]; then
      helpFunction
    fi

    OS=$1
    shift
    ;;
  esac
done

if [[ "$OS" != "macos" ]] && [[ "$OS" != "ios" ]]; then
  helpFunction
fi

if [[ "$OS" == "ios" ]]; then
  # Network-extension is the default for IOS
  NETWORKEXTENSION=1
fi

if ! [ -d "src" ] || ! [ -d "ios" ] || ! [ -d "macos" ]; then
  die "This script must be executed at the root of the repository."
fi

QMAKE=qmake
if [ "$OS" = "macos" ] && ! [ "$QT_MACOS_BIN" = "" ]; then
  QMAKE=$QT_MACOS_BIN/qmake
elif [ "$OS" = "ios" ] && ! [ "$QT_IOS_BIN" = "" ]; then
  QMAKE=$QT_IOS_BIN/qmake
fi

$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

printn Y "Retrieve the wireguard-go version... "
(cd macos/gobridge && go list -m golang.zx2c4.com/wireguard | sed -n 's/.*v\([0-9.]*\).*/#define WIREGUARD_GO_VERSION "\1"/p') > macos/gobridge/wireguard-go-version.h
print G "done."

printn Y "Cleaning the existing project... "
rm -rf mozillavpn.xcodeproj/ || die "Failed to remove things"
print G "done."

print Y "Importing translation files..."
python3 scripts/importLanguages.py $([[ "$PROD" ]] && echo "-p" || echo "") || die "Failed to import languages"

printn Y "Extract the project version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION"

MACOS_FLAGS="
  QTPLUGIN+=qsvg
  CONFIG-=static
  CONFIG+=balrog
  MVPN_MACOS=1
"

IOS_FLAGS="
  MVPN_IOS=1
"

printn Y "Mode: "
if [[ "$RELEASE" ]]; then
  print G "release"
  MODE="CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release"
else
  print G "debug"
  MODE="CONFIG+=debug CONFIG-=release CONFIG-=debug_and_release"
fi

printn Y "OS: "
print G "$OS"
if [ "$OS" = "macos" ]; then
  PLATFORM=$MACOS_FLAGS
elif [ "$OS" = "ios" ]; then
  PLATFORM=$IOS_FLAGS
else
  die "Why we are here?"
fi

PRODMODE=
INSPECTOR=
printn Y "Production mode: "
if [[ "$PROD" ]]; then
  print G yes
  PRODMODE="CONFIG+=production"
elif [ "$OS" = "macos" ]; then
  print G "no (inspector enabled)"
  INSPECTOR="CONFIG+=inspector"
else
  print G no
fi

VPNMODE=
printn Y "VPN mode: "
if [[ "$NETWORKEXTENSION" ]]; then
  print G network-extension
  VPNMODE="CONFIG+=networkextension"
else
  print G daemon
fi

print Y "Creating the xcode project via qmake..."
$QMAKE \
  VERSION=$SHORTVERSION \
  BUILD_ID=$FULLVERSION \
  -spec macx-xcode \
  $MODE \
  $PRODMODE \
  $INSPECTOR \
  $VPNMODE \
  $PLATFORM \
  src/src.pro || die "Compilation failed"

print Y "Patching the xcode project..."
ruby scripts/xcode_patcher.rb "MozillaVPN.xcodeproj" "$SHORTVERSION" "$FULLVERSION" "$OS" "$NETWORKEXTENSION" || die "Failed to merge xcode with wireguard"
print G "done."

print Y "Opening in XCode..."
open MozillaVPN.xcodeproj
print G "All done!"
