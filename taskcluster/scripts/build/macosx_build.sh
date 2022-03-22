#!/bin/bash

# Dependencies
python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m macos/qt-dependencies.tt.manifest
export PATH="`pwd`/qt/bin:$PATH"
export PATH="$(realpath ~/Library/Python/3.6/bin):$(realpath ~/.gem/ruby/2.6.0/bin):$PATH"
# install xcodeproj which is needed by xcode_patcher.rb
# use --user-install for permissions
gem install xcodeproj --user-install

# install rust
curl https://sh.rustup.rs -sSf | sh -s -- -y
export PATH="$HOME/.cargo/bin:$PATH"

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

# install python packages
# use --user for permissions
pip3 install -r requirements.txt --user

export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8
export PYTHONIOENCODING="UTF-8"

python3 scripts/utils/generate_glean.py
python3 scripts/utils/import_languages.py -m

curl -O https://dl.google.com/go/go1.17.6.darwin-amd64.tar.gz
tar -xzf go1.17.6.darwin-amd64.tar.gz
export PATH="`pwd`/go/bin:$PATH"

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
./scripts/macos/apple_compile.sh macos

make -f MozillaVPN.xcodeproj/qt_makeqmake.mak
make -f MozillaVPN.xcodeproj/qt_preprocess.mak
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO -project MozillaVPN.xcodeproj

# Package
python3 ./scripts/macos/import_pkg_resources.py

export BUILD=$UPLOAD_DIR
# Full path of the app in a temp folder
mkdir -p $BUILD
cp -r Release/Mozilla\ VPN.app $BUILD
# Copy the script
cp -r ./macos/pkg/scripts $BUILD
cp -r ./macos/pkg/Distribution $BUILD
cp -r ./macos/pkg/Resources $BUILD

cd $BUILD
zip -r unsigned.zip .
