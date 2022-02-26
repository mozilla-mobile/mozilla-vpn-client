#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi

print N "This script compiles and runs MozillaVPN tests on MacOS and Linux"
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

print Y "Cleaning up all..."
utest_cleanup_unit || die "Failed"
utest_cleanup_auth || die "Failed"
utest_cleanup_nativemessaging || die "Failed"

print Y "Installing dependencies..."
utest_dependencies || die "Failed"

print Y "Creating the xcode project for unit-tests via qmake..."
utest_qmake_unit || die "Failed"

print Y "Compile the unit-tests..."
utest_compile_unit || die "Failed"

print Y "Running the unit-tests..."
utest_run_unit || die "Failed"

printn Y "Cleaning the existing project... "
utest_cleanup_unit || die "Failed"

print Y "Creating the xcode project for auth-unit-tests via qmake..."
utest_qmake_auth || die "Failed"

print Y "Compile the auth-unit-tests..."
utest_compile_auth || die "Failed"

print Y "Running the unit-tests..."
utest_run_auth || die "Failed"

printn Y "Cleaning the existing project... "
utest_cleanup_auth || die "Failed"

print Y "Creating the xcode project for native-messaging-unit-tests via qmake..."
utest_qmake_nativemessaging || die "Failed"

print Y "Compile the native-messaging-unit-tests..."
utest_compile_nativemessaging || die "Failed"

print Y "Running the unit-tests..."
utest_run_nativemessaging || die "Failed"

printn Y "Cleaning the existing project... "
utest_cleanup_nativemessaging || die "Failed"

print G "All done!"
