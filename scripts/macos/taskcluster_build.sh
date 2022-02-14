#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../commons.sh

if [[ "$OS" != "macos" ]] && [[ "$OS" != "ios" ]] && [[ "$OS" != "macostest" ]]; then
  die "Wrong execution path"
fi

# Dependencies
python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m macos/qt-dependencies.tt.manifest || die
pip3 install -r requirements.txt || die

# Creating env file
cat > .env << EOF
QT_MACOS_BIN=`pwd`/qt/bin
EOF

# Creating the xcode.config file
echo "DEVELOPMENT_TEAM = 43AQ936H96" >> xcode.xconfig
echo "GROUP_ID_MACOS = group.org.mozilla.macos.Guardian" >> xcode.xconfig
echo "APP_ID_MACOS = org.mozilla.macos.FirefoxVPN" >> xcode.xconfig
echo "NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension" >> xcode.xconfig
echo "LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login" >> xcode.xconfig
echo "GROUP_ID_IOS = group.org.mozilla.ios.Guardian" >> xcode.xconfig
echo "APP_ID_IOS = org.mozilla.ios.FirefoxVPN" >> xcode.xconfig
echo "NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension" >> xcode.xconfig

# Let's start
./scripts/macos/apple_compile.sh macos --compile || die

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
