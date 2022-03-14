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

print N "This script compiles and runs MozillaVPN lottie tests on MacOS and Linux"
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

print Y "Compile the lottie unit-tests..."
lottie_compile_unit || die

print Y "Running the lottie unit-tests..."
lottie_run_unit || die

if [[ "$GRCOV_FILENAME" ]]; then
  printn Y "Generating temp coverage file for lottie unit tests..."  
  _grcov_lottie unit_"$GRCOV_FILENAME"
fi

printn Y "Cleaning the existing project... "
lottie_cleanup_unit || die

print Y "Compile the lottie QML tests..."
lottie_compile_qml || die

print Y "Running the lottie QML tests..."
lottie_run_qml || die

if [[ "$GRCOV_FILENAME" ]]; then
  printn Y "Generating temp coverage file for lottie qml tests...$GRCOV_FILENAME"  
  _grcov_lottie_qml qml_"$GRCOV_FILENAME"
fi

print Y "Cleaning the existing project... "
lottie_cleanup_qml || die

if [[ "$GRCOV_FILENAME" ]]; then  
  printn Y "merging temp files to lottie_lcov.info... "
  grcov -t lcov -o "$GRCOV_FILENAME" unit_"$GRCOV_FILENAME" qml_"$GRCOV_FILENAME" || die "merging temp files to final failed"

  printn Y "Cleaning the temp coverage files... "
  rm unit_"$GRCOV_FILENAME" qml_"$GRCOV_FILENAME" || die "cleaning cov files failed"
fi

print G "All done!"
