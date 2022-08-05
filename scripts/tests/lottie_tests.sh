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

if [[ "$OSTYPE" == "darwin"* ]]; then
  print N "Configure for darwin"
  . scripts/macos/utils/commons.sh
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
  print Y "Generating temp coverage file for lottie unit tests..."
  lottie_grcov_unit /tmp/lottie_unit.info || die
fi

print Y "Cleaning the existing project... "
lottie_cleanup_unit || die

print Y "Compile the lottie QML tests..."
lottie_compile_qml || die

print Y "Running the lottie QML tests..."
lottie_run_qml || die

if [[ "$GRCOV_FILENAME" ]]; then
  print Y "Generating temp coverage file for lottie qml tests..."
  lottie_grcov_qml /tmp/lottie_qml.info || die
fi

print Y "Cleaning the existing project... "
lottie_cleanup_qml || die

if [[ "$GRCOV_FILENAME" ]]; then
  print Y "merging temp files to $GRCOV_FILENAME... "
  grcov -t lcov -o "$GRCOV_FILENAME" \
    /tmp/lottie_unit.info \
    /tmp/lottie__qml.info || die "merging temp files to final failed"

  print Y "Cleaning the temp coverage files... "
  rm /tmp/lottie_unit.info /tmp/lottie_qml.info || die "cleaning cov files failed"
fi

print G "All done!"
