#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../commons.sh

if [ -f .env ]; then
  . .env
fi

print N "This script compiles and runs MozillaVPN QML tests on MacOS and Linux"
print N ""

if ! [ -d "src" ] || ! [ -d "tests" ]; then
  die "This script must be executed at the root of the repository."
fi

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  print N "Configure for linux"
  . scripts/linux/utils/commons.sh
elif [[ "$OSTYPE" == "darwin"* ]]; then
  print N "Configure for darwin"
  . scripts/macos/utils/commons.sh
elif [[ "$OSTYPE" == "msys"* ]]; then
  print N "Configure for windows"
  . scripts/windows/utils/commons.sh
else
  die "Unsupported platform (yet?)"
fi

print Y "Cleaning the existing project... "
qmltest_cleanup || die

print Y "Creating the xcode project via qmake for QML tests..."
qmltest_qmake || die

print Y "Compile the QML tests..."
qmltest_compile || die

print Y "Running the QML tests..."
qmltest_run || die

printn Y "Cleaning the existing project... "
qmltest_cleanup || die

print G "All done!"
