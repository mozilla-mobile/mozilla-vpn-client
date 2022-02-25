#!/bin/bash

# Dependencies
python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m macos/qt-dependencies.tt.manifest
export PATH="`pwd`/qt/bin:$PATH"
# TODO: internal pypi mirror, if it doesn't get used by default?
pip3 install glean_parser==3.5

cd macos/gobridge
go list -m golang.zx2c4.com/wireguard | sed -n 's/.*v\([0-9.]*\).*/#define WIREGUARD_GO_VERSION "\1"/p') > macos/gobridge/wireguard-go-version.h
cd ../..

# Xcode config - maybe we should just maintain a full Xcode config somewhere instead of replacing things here?
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
echo "$SHORTVERSION - $FULLVERSION"
echo "DEVELOPMENT_TEAM = 43AQ936H96" >> xcode.xconfig
echo "GROUP_ID_MACOS = group.org.mozilla.macos.Guardian" >> xcode.xconfig
echo "APP_ID_MACOS = org.mozilla.macos.FirefoxVPN" >> xcode.xconfig
echo "NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension" >> xcode.xconfig
echo "LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login" >> xcode.xconfig
echo "GROUP_ID_IOS = group.org.mozilla.ios.Guardian" >> xcode.xconfig
echo "APP_ID_IOS = org.mozilla.ios.FirefoxVPN" >> xcode.xconfig
echo "NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension" >> xcode.xconfig
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
  CONFIG+=balrog \
  MVPN_MACOS=1 \
  src/src.pro
ruby scripts/macos/utils/xcode_patcher.rb \
  "MozillaVPN.xcodeproj" \
  "$SHORTVERSION" \
  "$FULLVERSION" \
  macos \
  Qt5

# Build
python3 scripts/utils/generate_glean.py
python3 scripts/utils/import_languages.py -m
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -project MozillaVPN.xcodeproj

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
