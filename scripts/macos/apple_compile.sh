#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi

RELEASE=1
OS=
ADJUST_SDK_TOKEN=
ADJUST="CONFIG-=adjust"
QTBINPATH=

helpFunction() {
  print G "Usage:"
  print N "\t$0 <macos|ios|> [-d|--debug] [-a|--adjusttoken <adjust_token>] [-q|--qtbin <qtbinpath>]"
  print N ""
  print N "By default, the project is compiled in release mode. Use -d or --debug for a debug build."
  print N ""
  print N "If MVPN_IOS_ADJUST_TOKEN env is found, this will be used at compilation time."
  print N ""
  print G "Config variables:"
  print N "\tQT_MACOS_BIN=</path/of/the/qt/bin/folder/for/macos>"
  print N "\tQT_IOS_BIN=</path/of/the/qt/bin/folder/for/ios>"
  print N "\tMVPN_IOS_ADJUST_TOKEN=<token>"
  print N ""
  exit 0
}

print N "This script compiles MozillaVPN for MacOS/iOS"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -a | --adjusttoken)
    ADJUST_SDK_TOKEN="$2"
    shift
    shift
    ;;
  -d | --debug)
    RELEASE=
    shift
    ;;
  -q | --qtbinpath)
    QTBINPATH="$2"
    shift
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

if ! [[ "$ADJUST_SDK_TOKEN" ]] && [[ "$MVPN_IOS_ADJUST_TOKEN" ]]; then
  print Y "Using the MVPN_IOS_ADJUST_TOKEN value for the adjust token"
  ADJUST_SDK_TOKEN=$MVPN_IOS_ADJUST_TOKEN
fi

if ! [ -d "src" ] || ! [ -d "ios" ] || ! [ -d "macos" ]; then
  die "This script must be executed at the root of the repository."
fi

QT_BIN=
if [ "$OS" = "macos" ] && ! [ "$QT_MACOS_BIN" = "" ]; then
  QT_BIN=$QT_MACOS_BIN
elif [ "$OS" = "ios" ] && ! [ "$QT_IOS_BIN" = "" ]; then
  QT_BIN=$QT_IOS_BIN
fi

QMAKE="$QT_BIN/qmake"
print G "qmake path: $QMAKE"
$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

export PATH="$QT_BIN:$PATH"

if [[ "$OS" == "ios" ]]; then
  printn Y "Retrieve the wireguard-go version... "
  (cd ios/gobridge && go list -m golang.zx2c4.com/wireguard | sed -n 's/.*v\([0-9.]*\).*/#define WIREGUARD_GO_VERSION "\1"/p') > ios/gobridge/wireguard-go-version.h
  print G "done."
fi

printn Y "Cleaning the existing project... "
rm -rf mozillavpn.xcodeproj/ || die "Failed to remove things"
print G "done."

print Y "Importing translation files..."
python3 scripts/utils/import_languages.py $([[ $QTBINPATH ]] && echo "-q $QTBINPATH") $([[ "$OS" = "macos" ]] && echo "-m" || echo "") || die "Failed to import languages"

print Y "Generating glean samples..."
python3 scripts/utils/generate_glean.py || die "Failed to generate glean samples"

printn Y "Extract the project version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION"

MACOS_FLAGS="
  QTPLUGIN+=qsvg
  CONFIG-=static
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

OSRUBY=$OS
printn Y "OS: "
print G "$OS"
if [ "$OS" = "macos" ]; then
  PLATFORM=$MACOS_FLAGS
elif [ "$OS" = "ios" ]; then
  PLATFORM=$IOS_FLAGS
  if [[ "$ADJUST_SDK_TOKEN"  ]]; then
    printn Y "ADJUST_SDK_TOKEN: "
    print G "$ADJUST_SDK_TOKEN"
    ADJUST="CONFIG+=adjust"
  fi
else
  die "Why we are here?"
fi

print Y "Creating the xcode project via qmake..."
$QMAKE \
  VERSION=$SHORTVERSION \
  BUILD_ID=$FULLVERSION \
  -spec macx-xcode \
  $MODE \
  $PLATFORM \
  $ADJUST \
  src/src.pro || die "Compilation failed"

PROJECT="Mozilla VPN.xcodeproj"
[[ "$OS" = "ios" ]] && PROJECT="MozillaVPN.xcodeproj"

print Y "Patching the xcode project..."
ruby scripts/macos/utils/xcode_patcher.rb "$PROJECT" "$SHORTVERSION" "$FULLVERSION" "$OSRUBY" "$ADJUST_SDK_TOKEN" || die "Failed to merge xcode with wireguard"
print G "done."


if command -v "sed" &>/dev/null; then
  sed -i '' '/<key>BuildSystemType<\/key>/d' "$PROJECT/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings"
  sed -i '' '/<string>Original<\/string>/d' "$PROJECT/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings"
fi

print Y "Opening in XCode..."
open "$PROJECT"
print G "All done!"
