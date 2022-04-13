#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../../../scripts/utils/commons.sh

print N "Taskcluster macOS compilation script"
print N ""

export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8

print Y "Installing ruby dependencies..."
# use --user-install for permissions
gem install xcodeproj --user-install || die
export PATH="$HOME/.gem/ruby/2.6.0/bin:$PATH"

print Y "Installing rust..."
curl https://sh.rustup.rs -sSf | sh -s -- -y || die
export PATH="$HOME/.cargo/bin:$PATH"

print Y "Installing go..."
curl -O https://dl.google.com/go/go1.17.6.darwin-amd64.tar.gz
tar -xzf go1.17.6.darwin-amd64.tar.gz
export PATH="`pwd`/go/bin:$PATH"

print Y "Installing python dependencies..."
# use --user for permissions
python3 -m pip install -r requirements.txt --user
export PYTHONIOENCODING="UTF-8"

print Y "Installing QT..."
# generate qt_static_macos
auth_header="$(git config --local --get http.https://github.com/.extraheader)"
git clone https://github.com/mozilla-mobile/qt_static_macos --depth 1 || die
cd qt_static_macos || die
cat qt6* > qt_static.tar.gz
tar xf qt_static.tar.gz || die

cat > qt6/bin/qt.conf << EOF
[Paths]
Prefix=`pwd`/qt6
EOF

cp qt6/bin/qt.conf qt6/libexec || die
cd ..
export QT_MACOS_BIN=`pwd`/qt_static_macos/qt6/bin
export PATH=`pwd`/qt_static_macos/qt6/bin:$PATH

print Y "Updating submodules..."
# should already be done by XCode cloud cloning but just to make sure
git submodule init || die
git submodule update || die

print Y "Configuring the build..."
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
./scripts/macos/apple_compile.sh macos || die

print Y "Compiling..."
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO -project Mozilla\ VPN.xcodeproj || die


print Y "Creating the final package..."
python3 ./scripts/macos/import_pkg_resources.py || die

set
print Y "Exporting the artifact..."
mkdir -p ../../artifacts || die
mkdir -p tmp || die
cp -r Release/Mozilla\ VPN.app tmp || die
cp -r ./macos/pkg/scripts tmp || die
cp -r ./macos/pkg/Distribution tmp || die
cp -r ./macos/pkg/Resources tmp || die
cd tmp || die

zip -r ../../../artifacts/unsigned_artifact.zip . || die
cd .. || die
rm -rf tmp || die

print G "Done!"
