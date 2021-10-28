#!/bin/sh

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# switch to repository directory for setup
cd /Volumes/workspace/repository

# make sure submodules are up to date
# should already be done by XCode cloud cloning but just to make sure
git submodule init
git submodule update

# generate qt_ios
git clone https://github.com/mbirghan/qt_ios_build
cd qt_ios_build
cat x* > qt_static.tar.gz
tar xf qt_static.tar.gz
cd ..

export QT_IOS_BIN=`pwd`/qt_ios_build/ios/bin

# add necessary directories to path
export PATH=`pwd`/qt_ios_build/ios/bin:/Users/local/.gem/ruby/2.6.0/bin:/Users/local/Library/Python/3.8/bin:$PATH

# install xcodeproj which is needed by xcode_patcher.rb
# use --user-install for permissions
gem install xcodeproj --user-install

# install python packages
# use --user for permissions
pip3 install "glean_parser==3.5" --user
pip3 install pyhumps --user
pip3 install pyyaml --user
python3 scripts/generate_glean.py
python3 scripts/importLanguages.py -m

# install go and set GOROOT using brew
# wget is currently not supported so we need to use brew
brew install go

# create xcode.xconfig
cat > xcode.xconfig << EOF
DEVELOPMENT_TEAM = 43AQ936H96
GROUP_ID_MACOS = group.org.mozilla.macos.Guardian
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
EOF

./scripts/apple_compile.sh ios

# use New Build System instead of legacy build system
cat > ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
  <dict>
    <key>IDEWorkspaceSharedSettings_AutocreateContextsIfNeeded</key>
    <false/>
  </dict>
</plist>
EOF

# build Qt resources
# XCode Cloud has some problem with dependencies and timing therefore we have to
# build Qt before we call xcodebuild
make -f MozillaVPN.xcodeproj/qt_makeqmake.mak
make -f MozillaVPN.xcodeproj/qt_preprocess.mak

