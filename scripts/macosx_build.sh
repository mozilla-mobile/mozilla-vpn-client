#!/bin/bash

# Dependencies
python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m macos/qt-dependencies.tt.manifest
export PATH="`pwd`/qt/bin:$PATH"
# install xcodeproj which is needed by xcode_patcher.rb
# use --user-install for permissions
gem install xcodeproj --user-install

# make sure submodules are up to date
# should already be done by XCode cloud cloning but just to make sure
git submodule init
git submodule update

# generate qt_static_macos
auth_header="$(git config --local --get http.https://github.com/.extraheader)"
git clone https://github.com/mozilla-mobile/qt_static_macos
cd qt_static_macos
cat x* > qt_static.tar.gz
tar xf qt_static.tar.gz
cd ..
export QT_MACOS_BIN=`pwd`/qt_static_macos/qt/bin
export PATH=`pwd`/qt_static_macos/qt/bin:$PATH

export PATH="/Users/task_163336061065616/Library/Python/3.6/bin:/Users/task_163336061065616/.gem/ruby/2.6.0/bin:$PATH"

# install python packages
# use --user for permissions
pip3 install "glean_parser==3.5" --user
pip3 install pyhumps --user
pip3 install pyyaml --user

export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8
export PYTHONIOENCODING="UTF-8"  

python3 scripts/generate_glean.py
python3 scripts/importLanguages.py -m

# cd macos/gobridge
# (go list -m golang.zx2c4.com/wireguard | sed -n 's/.*v\([0-9.]*\).*/#define WIREGUARD_GO_VERSION "\1"/p') > macos/gobridge/wireguard-go-version.h
# cd ../..

git clone https://go.googlesource.com/go goroot
cd goroot
git checkout go1.17.1
cd src
./all.bash
export PATH="`pwd`/goroot/bin:$PATH"
echo `go version`
echo `ls -ls ./goroot/bin`
echo $PATH
cd ../..


echo Y "Printing go path"
echo `which go`

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
./scripts/apple_compile.sh macos
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -project MozillaVPN.xcodeproj

# Package
python3 ./scripts/import_macos_pkg_resources.py
# Full path of the app in a temp folder
mkdir -p BUILD
cp -r Release/Mozilla\ VPN.app BUILD
# Copy the script
cp -r ./macos/pkg/scripts BUILD
cp -r ./macos/pkg/Distribution BUILD
cp -r ./macos/pkg/Resources BUILD

cd BUILD
zip -r ../build/src/artifacts/unsigned.zip .
