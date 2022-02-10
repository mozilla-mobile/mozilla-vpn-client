#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

if [ -f .env ]; then
  . .env
fi

print N "This script compiles MozillaVPN tests for MacOS"
print N ""

if ! [ -d "src" ] || ! [ -d "ios" ] || ! [ -d "macos" ]; then
  die "This script must be executed at the root of the repository."
fi

QT_BIN=$QT_MACOS_BIN
QMAKE="$QT_BIN/qmake"
$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

export PATH="$QT_BIN:$PATH"

printn Y "Cleaning the existing project... "
rm -rf tests.xcodeproj/ || die "Failed to remove things"
rm -rf .qm .moc .obj .rcc || die "Failed"
print G "done."

print Y "Creating the xcode project via qmake for lottie unit-tests..."
$QMAKE \
  -spec macx-xcode \
  QTPLUGIN+=qsvg \
  MVPN_MACOS=1 \
  CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release \
  CONFIG+=sdk_no_version_check \
  CONFIG+=coverage \
  lottie/tests/unit/unit.pro || die "Compilation failed"

print Y "Compile the lottie unit-tests..."
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -enableCodeCoverage YES -project tests.xcodeproj || die "Failed"
print G "All done!"

print Y "Running the lottie unit-tests..."
./Release/tests.app/Contents/MacOS/tests || die "Failed"
print G "All done!"

printn Y "Cleaning the existing project... "
rm -rf tst_lottie.xcodeproj/ || die "Failed to remove things"
rm -rf .qm .moc .obj .rcc || die "Failed"
print G "done."

print Y "Creating the xcode project via qmake for lottie unit-tests..."
$QMAKE \
  -spec macx-xcode \
  QTPLUGIN+=qsvg \
  MVPN_MACOS=1 \
  CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release \
  CONFIG+=sdk_no_version_check \
  CONFIG+=coverage \
  lottie/tests/qml/qml.pro || die "Compilation failed"

print Y "Compile the qml unit-tests..."
xcodebuild build CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -project tst_lottie.xcodeproj || die "Failed"
print G "All done!"

print Y "Running the qml unit-tests..."
./Release/tst_lottie.app/Contents/MacOS/tst_lottie || die "Failed"
print G "All done!"
