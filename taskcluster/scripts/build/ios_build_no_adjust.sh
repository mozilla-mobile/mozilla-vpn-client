#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e


. $(dirname $0)/../../../scripts/utils/commons.sh

print N "Taskcluster iOS compilation script"
print N ""

export LC_ALL=en_US.utf-8
export LANG=en_US.utf-8

print Y "Installing rust..."
curl https://sh.rustup.rs -sSf | sh -s -- -y || die
export PATH="$HOME/.cargo/bin:$PATH"
rustup target add x86_64-apple-ios aarch64-apple-ios

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
QTVERSION=$(ls ${MOZ_FETCHES_DIR}/qt_ios)
echo "Using QT:$QTVERSION"

print Y "Installing Homebrew..."
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

print Y "Installing cmake..."
brew install cmake

print Y "Get the submodules..."
git submodule update --init --depth 1 || die "Failed to init submodules"
# Technically we do not need the following line because we later call the
# apple compile script which calls import languages. However when we move to cmake
# for iOS we can remove the call to import languages and this step will be necessary
for i in src/apps/*/translations/i18n; do
  git submodule update --remote $i || die "Failed to pull latest i18n from remote ($i)"
done
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
# ./scripts/macos/apple_compile.sh ios -q ../../fetches/qt_ios/$QTVERSION/macos/bin -a ReallyNotAnAPIToken || die
print Y "Building xcodeproj..."
../../fetches/qt_ios/$QTVERSION/ios/bin/qt-cmake -S . -B ${MOZ_FETCHES_DIR}/build -GNinja

print Y "Compiling..."
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO -project ${MOZ_FETCHES_DIR}/build/Mozilla\ VPN.xcodeproj || die

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
