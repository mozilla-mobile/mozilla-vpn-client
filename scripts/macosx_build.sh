#!/bin/bash

# Dependencies
python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m macos/qt-dependencies.tt.manifest
export PATH="`pwd`/qt/bin:$PATH"
# TODO: internal pypi mirror, if it doesn't get used by default?
pip3 install glean_parser==3.5

# Xcode config - maybe we should just maintain a full Xcode config somewhere instead of replacing things here?
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
echo "$SHORTVERSION - $FULLVERSION"
echo "APP_ID_MACOS = org.mozilla.macos.FirefoxVPN" >> xcode.xconfig
echo "LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login" >> xcode.xconfig
echo "GROUP_ID_IOS = group.org.mozilla.ios.Guardian" >> xcode.xconfig
qmake \
  VERSION=$SHORTVERSION \
  BUILD_ID=$FULLVERSION \
  -spec macx-xcode \
  CONFIG-=debug \
  CONFIG-=debug_and_release \
  CONFIG+=release \
  CONFIG+=production \
  QTPLUGIN+=qsvg \
  CONFIG-=static \
  MVPN_MACOS=1 \
  src/src.pro
ruby scripts/macos/utils/xcode_patcher.rb \
  "Mozilla VPN.xcodeproj" \
  "$SHORTVERSION" \
  "$FULLVERSION" \
  macos \
  Qt5

# Build
python3 scripts/utils/import_languages.py -m
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -project "Mozilla VPN.xcodeproj"

# Package
python3 ./scripts/macos/import_pkg_resources.py
# Full path of the app in a temp folder
mkdir -p BUILD
cp -r Release/Mozilla\ VPN.app BUILD
# Copy the script
cp -r ./macos/pkg/scripts BUILD
cp -r ./macos/pkg/Distribution BUILD
cp -r ./macos/pkg/Resources BUILD

cd BUILD
zip -r ../build/src/artifacts/unsigned.zip .
