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
NETWORKEXTENSION=
WEBEXTENSION=

helpFunction() {
  print G "Usage:"
  print N "\t$0 <macos|ios|macostest> [-d|--debug] [-n|--networkextension] [-w|--webextension]"
  print N ""
  print N "By default, the project is compiled in release mode. Use -d or --debug for a debug build."
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
  -n | --networkextension)
    NETWORKEXTENSION=1
    shift
    ;;
  -w | --webextension)
    WEBEXTENSION=1
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

if [[ "$OS" != "macos" ]] && [[ "$OS" != "ios" ]] && [[ "$OS" != "macostest" ]]; then
  helpFunction
fi

if [[ "$OS" == "ios" ]]; then
  # Network-extension is the default for IOS
  NETWORKEXTENSION=1
  # No web-extension for IOS
  WEBEXTENSION=

  command -v wget >/dev/null 2>&1 || die "wget must be installed to download the Adjust SDK"
  command -v unzip >/dev/null 2>&1 || die "unzip must be installed to unzip the Adjust SDK"
  command -v sha256Sum >/dev/null 2>&1 || die "sha256sum must be installed to verify the Adjust SDK"

  if ! [ -d "3rdparty/AdjustSdk.framework" ]; then
    rm -f 3rdparty/AdjustSdkDynamic.framework.zip
    wget -nc -O 3rdparty/AdjustSdkDynamic.framework.zip https://github.com/adjust/ios_sdk/releases/download/v4.29.4/AdjustSdkDynamic.framework.zip || die "wget for the Adjust SDK failed"

    if ! sha256sum "3rdparty/AdjustSdkDynamic.framework.zip" | grep -q "31151c89315b424ab0e39980502e8b4596d4cbb89bfe38a0a1ce09d3d67a32f4  3rdparty/AdjustSdkDynamic.framework.zip"; then
      rm -f 3rdparty/AdjustSdkDynamic.framework.zip
      die "Error while downloading please try again"
    fi

    unzip 3rdparty/AdjustSdkDynamic.framework.zip -d 3rdparty/ || die "unzipping the Adjust SDK failed"
    mv -n 3rdparty/AdjustSdkDynamic/AdjustSdk.framework 3rdparty/AdjustSdk.framework
    rm -rf 3rdparty/AdjustSdkDynamic
  fi

fi

if ! [ -d "src" ] || ! [ -d "ios" ] || ! [ -d "macos" ]; then
  die "This script must be executed at the root of the repository."
fi

QMAKE=qmake
if [ "$OS" = "macos" ] && ! [ "$QT_MACOS_BIN" = "" ]; then
  QMAKE=$QT_MACOS_BIN/qmake
elif [ "$OS" = "macostest" ] && ! [ "$QT_MACOS_BIN" = "" ]; then
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
git submodule update --remote --depth 1 i18n || die "Failed to fetch newest translation files"
python3 scripts/importLanguages.py $([[ "$OS" = "macos" ]] && echo "-m" || echo "") || die "Failed to import languages"

print Y "Generating glean samples..."
python3 scripts/generate_glean.py || die "Failed to generate glean samples"

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

MACOSTEST_FLAGS="
  QTPLUGIN+=qsvg
  CONFIG-=static
  CONFIG+=DUMMY
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

OSRUBY=$OS
printn Y "OS: "
print G "$OS"
if [ "$OS" = "macos" ]; then
  PLATFORM=$MACOS_FLAGS
elif [ "$OS" = "macostest" ]; then
  OSRUBY=macos
  PLATFORM=$MACOSTEST_FLAGS
elif [ "$OS" = "ios" ]; then
  PLATFORM=$IOS_FLAGS
else
  die "Why we are here?"
fi

VPNMODE=
printn Y "VPN mode: "
if [[ "$NETWORKEXTENSION" ]]; then
  print G network-extension
  VPNMODE="CONFIG+=networkextension"
else
  print G daemon
fi

printn Y "Web-Extension: "
WEMODE=
if [[ "$WEBEXTENSION" ]]; then
  print G web-extension
  WEMODE="CONFIG+=webextension"
else
  print G daemon
fi

print Y "Creating the xcode project via qmake..."
$QMAKE \
  VERSION=$SHORTVERSION \
  BUILD_ID=$FULLVERSION \
  -spec macx-xcode \
  $MODE \
  $VPNMODE \
  $WEMODE \
  $PLATFORM \
  src/src.pro || die "Compilation failed"

print Y "Patching the xcode project..."
ruby scripts/xcode_patcher.rb "MozillaVPN.xcodeproj" "$SHORTVERSION" "$FULLVERSION" "$OSRUBY" "$NETWORKEXTENSION" "$WEBEXTENSION"|| die "Failed to merge xcode with wireguard"
print G "done."

print Y "Opening in XCode..."
open MozillaVPN.xcodeproj
print G "All done!"
