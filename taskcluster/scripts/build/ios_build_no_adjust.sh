#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
set -x

. $(dirname $0)/../../../scripts/utils/commons.sh

print N "Taskcluster iOS compilation script"
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
PROJECT_HOME=`pwd`
QTVERSION=$(ls ../../fetches/qt_ios)
echo "Using QT:$QTVERSION"

cd ../../fetches/qt_ios/$QTVERSION/ios || die
export QT_IOS_BIN=`pwd`/bin
export PATH=$QT_IOS_BIN:$PATH

cd $PROJECT_HOME
# So ios qmake is just a wrapper script
# and expects to find pwd/qt_ios/mac/bin/qmake >:c
ln -s ../../fetches/qt_ios/ qt_ios

print Y "Get the submodules..."
git submodule update --init --depth 1 || die "Failed to init submodules"
# Technically we do not need the following line because we later call the
# apple compile script which calls import languages. However when we move to cmake
# for iOS we can remove the call to import languages and this step will be necessary
git submodule update --remote i18n || die "Failed to pull latest i18n from remote"
print G "done."

print Y "Configuring the build..."

# create xcode.xconfig
cat > xcode.xconfig << EOF
APP_ID_MACOS = org.mozilla.macos.FirefoxVPN
LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item
GROUP_ID_IOS = group.org.mozilla.ios.Guardian
APP_ID_IOS = org.mozilla.ios.FirefoxVPN
NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension
EOF

# NOTE: In case we want to release this, we need to get that token, see android-release.sh
./scripts/macos/apple_compile.sh ios -q ../../fetches/qt_ios/$QTVERSION/macos/bin -a ReallyNotAnAPIToken || die

print Y "Compiling..."
# TODO: apple_compile.sh ios generated "MozillaVPN.xcodeproj"; the mac one generated "Mozilla VPN.xcodeproj"
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO -project MozillaVPN.xcodeproj || die

print Y "Exporting the artifact..."
mkdir -p tmp || die
cp -r Release-iphoneos/* tmp || die
cd tmp || die

# From checkout dir to actual task base directory
TASK_HOME=$(dirname "${MOZ_FETCHES_DIR}" )
rm -rf "${TASK_HOME}/artifacts"
mkdir -p "${TASK_HOME}/artifacts"
tar -czvf "${TASK_HOME}/artifacts/MozillaVPN.tar.gz" . || die
cd .. || die
rm -rf tmp || die

print G "Done!"
