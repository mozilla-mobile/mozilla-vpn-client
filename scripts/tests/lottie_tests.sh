#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../commons.sh

if [ -f .env ]; then
  . .env
fi

print N "This script compiles and runs MozillaVPN lottie tests on MacOS and Linux"
print N ""

if ! [ -d "src" ] || ! [ -d "lottie" ]; then
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
lottie_cleanup_unit || die
lottie_cleanup_qml || die

print Y "Creating the xcode project via qmake for lottie unit-tests..."
lottie_qmake_unit || die

print Y "Compile the lottie unit-tests..."
lottie_compile_unit || die

print Y "Running the lottie unit-tests..."
lottie_run_unit || die

printn Y "Cleaning the existing project... "
lottie_cleanup_unit || die

print Y "Creating the xcode project via qmake for lottie qml-tests..."
lottie_qmake_qml || die

print Y "Compile the lottie QML tests..."
lottie_compile_qml || die

print Y "Running the lottie QML tests..."
lottie_run_qml || die

print Y "Cleaning the existing project... "
lottie_cleanup_qml || die

print G "All done!"
