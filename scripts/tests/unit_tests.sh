#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi

helpFunction() {
  print G "Usage:"
  print N "\t$0 [-g|--grcov <grcov.info>]"
  print N ""
  print N "Use -g or --grcov to name the code coverage output"
  print N ""
  exit 0
}

print N "This script compiles and runs MozillaVPN tests on MacOS and Linux"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -g | --grcov)
    GRCOV_FILENAME="$2"
    shift
    shift
    ;;
  *)
    helpFunction
    ;;
  esac
done

if ! [ -d "src" ] || ! [ -d "tests" ]; then
  die "This script must be executed at the root of the repository."
fi

if ! [[ "$MVPN_OATHTOOL" ]]; then
  die "Please set env var MVPN_OATHTOOL."
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

print Y "Compile the unit-tests..."
utest_compile_unit || die "Failed"

print Y "Running the unit-tests..."
utest_run_unit || die "Failed"

if [[ "$GRCOV_FILENAME" ]]; then
  print Y "Generating temp coverage file for unit tests..."
  utest_grcov_unit /tmp/utest_unit.info
fi

print Y "Cleaning the existing unit project... "
utest_cleanup_unit || die "Failed"

print Y "Compile the auth-unit-tests..."
utest_compile_auth || die "Failed"

print Y "Running the auth unit-tests..."
utest_run_auth || die "Failed"

if [[ "$GRCOV_FILENAME" ]]; then
  print Y "Generating temp coverage file for auth tests..."
  utest_grcov_auth /tmp/utest_auth.info
fi

print Y "Cleaning the existing auth project... "
utest_cleanup_auth || die "Failed"

print Y "Compile the native-messaging-unit-tests..."
utest_compile_nativemessaging || die "Failed"

print Y "Running the native messaging unit-tests..."
utest_run_nativemessaging || die "Failed"

if [[ "$GRCOV_FILENAME" ]]; then
  print Y "Generating temp coverage file for native messaging unit tests..."
  utest_grcov_nativemessaging /tmp/utest_nativemessaging.info
fi

print Y "Cleaning the existing native messaging project... "
utest_cleanup_nativemessaging || die "Failed"

if [[ "$GRCOV_FILENAME" ]]; then
  print Y "merging temp files to $GRCOV_FILENAME... "
  grcov -t lcov -o "$GRCOV_FILENAME" \
    /tmp/utest_unit.info \
    /tmp/utest_auth.info \
    /tmp/utest_nativemessaging.info || die "merging temp files to final failed"

  print Y "Cleaning the temp coverage files... "
  rm /tmp/utest_unit.info  /tmp/utest_auth.info /tmp/utest_nativemessaging.info || die "cleaning cov files failed"
fi

print G "All done!"
