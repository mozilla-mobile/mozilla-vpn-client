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
  print N "\t$0 <macos|ios|macostest> [-d|--debug] [-n|--networkextension] [-a|--adjusttoken <adjust_token>]"
  print N ""
  print N "By default, the project is compiled in release mode. Use -d or --debug for a debug build."
  print N "Use -n or --networkextension to force the network-extension component for MacOS too."
  print N ""
  print N "If MVPN_IOS_ADJUST_TOKEN env is found, this will be used at compilation time."
  print N ""
  print G "Config variables:"
  print N "\tQT_MACOS_BIN=</path/of/the/qt/bin/folder/for/macos>"
  print N "\tQT_IOS_BIN=</path/of/the/qt/bin/folder/for/ios>"
  print N "\tMVPN_IOS_ADJUST_TOKEN=<token>"
  print N ""
  exit 0
}

print N "This script compiles and runs MozillaVPN tests on MacOS and Linux"
print N ""

while [[ $# -gt 0 ]]; do  
  key="$1"

  print B "inside the while $1"
  case $key in
  -g | --grcov)
    print B "inside the case --grcov $1"
    GRCOV_FILENAME="$2"
    shift
    shift
    ;;
  -h | --help)
    helpFunction
    ;;   
  esac
done

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

print Y "Compile the unit-tests..."
utest_compile_unit || die "Failed"

print Y "Running the unit-tests..."
utest_run_unit || die "Failed"

if [[ "$GRCOV_FILENAME" ]]; then
  printn Y "Generating temp coverage file for unit tests..."
  utest_grcov_unit "$GRCOV_FILENAME"_unit || die
fi

printn Y "Cleaning the existing project... "
utest_cleanup_unit || die "Failed"

print Y "Compile the auth-unit-tests..."
utest_compile_auth || die "Failed"

print Y "Running the unit-tests..."
utest_run_auth || die "Failed"

if [[ "$GRCOV_FILENAME" ]]; then
  printn Y "Generating temp coverage file for auth tests..."
  utest_grcov_auth "$GRCOV_FILENAME"_auth || die
fi

printn Y "Cleaning the existing project... "
utest_cleanup_auth || die "Failed"

print Y "Compile the native-messaging-unit-tests..."
utest_compile_nativemessaging || die "Failed"

print Y "Running the unit-tests..."
utest_run_nativemessaging || die "Failed"

if [[ "$GRCOV_FILENAME" ]]; then
  printn Y "Generating temp coverage file for native messaging unit tests..."
  utest_grcov_nativemessaging "$GRCOV_FILENAME"_nativemessaging || die
fi

printn Y "Cleaning the existing project... "
utest_cleanup_nativemessaging || die "Failed"

if [[ "$GRCOV_FILENAME" ]]; then  
  printn Y "merging temp files to $GRCOV_FILENAME... "
  grcov -t lcov -o "$GRCOV_FILENAME" "$GRCOV_FILENAME"_auth "$GRCOV_FILENAME"_unit "$GRCOV_FILENAME"_nativemessaging || die

  printn Y "Cleaning the temp coverage files... "
  rm "$GRCOV_FILENAME"_auth "$GRCOV_FILENAME"_nativemessaging "$GRCOV_FILENAME"_unit || die
fi



print G "All done!"
