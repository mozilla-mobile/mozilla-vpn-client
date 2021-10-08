#!/bin/sh

#  ci_post_clone.sh
#  MozillaVPN
#
#  Created by mozilla on 29.07.21.
#  

cd /Volumes/workspace/repository

git clone https://github.com/mbirghan/qt_ios_build

export PATH=`pwd`/qt_ios_build/ios/bin:/Users/local/.gem/ruby/2.6.0/bin:$PATH
gem install xcodeproj --user-install

pip3 install "glean_parser==3.5" --user
pip3 install pyhumps --user
pip3 install pyyaml --user
python3 scripts/generate_glean.py

brew install go

export QT_IOS_BIN=`pwd`/qt_ios_build/ios/bin

git submodule update --remote --depth 1 i18n
python3 scripts/importLanguages.py -m

echo "DEVELOPMENT_TEAM = 43AQ936H96" >> xcode.xconfig
echo "GROUP_ID_MACOS = group.org.mozilla.macos.Guardian" >> xcode.xconfig
echo "APP_ID_MACOS = org.mozilla.macos.FirefoxVPN" >> xcode.xconfig
echo "NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension" >> xcode.xconfig
echo "LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item" >> xcode.xconfig
echo "GROUP_ID_IOS = group.org.mozilla.ios.Guardian" >> xcode.xconfig
echo "APP_ID_IOS = org.mozilla.ios.FirefoxVPN" >> xcode.xconfig
echo "NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension" >> xcode.xconfig

./scripts/apple_compile.sh ios

