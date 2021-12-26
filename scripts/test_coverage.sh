#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

REPORT_FILE_UNIT=/tmp/report_unit.html
REPORT_FILE_AUTH=/tmp/report_auth.html
LANGUAGE=en
LANG=en

JOBS=1

helpFunction() {
  print G "Usage:"
  print N "\t$0 [-j|--jobs <jobs>]"
  print N ""
  exit 0
}

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -j | --jobs)
    JOBS="$2"
    shift
    shift
    ;;
  -h | --help)
    helpFunction
    ;;
  *)
    helpFunction
    ;;
  esac
done

print N "This script runs the unit tests and shows the test coverage."
print N ""

if ! [ -d "src" ] || ! [ -d "tests" ]; then
  die "This script must be executed at the root of the repository."
fi

print Y "Compiling..."
make -j $JOBS || die "Failed to compile"
print G "done."

export LLVM_PROFILE_FILE=/tmp/mozillavpn.llvm

print Y "Running the unit-tests..."
./tests/unit/tests || die "Failed to run tests"
print G "done."

if ! [ -f $LLVM_PROFILE_FILE ]; then
  die "No report generated!"
fi

unset LLVM_PROFILE_FILE

print Y "Running the native-messaging tests..."
./tests/nativemessaging/tests ./extension/app/mozillavpnnp || die "Failed to run tests"
print G "done."

print Y "Running the lottie tests..."
./lottie/tests/unit/tests || die "Failed to run tests"
print G "done."

printn Y "(unit) Merge the profile data... "
llvm-profdata-10 merge /tmp/mozillavpn.llvm -o /tmp/mozillavpn.llvm-final || die "Failed to merge the coverage report"
print G "done."

print Y "(unit) Report:"
llvm-cov-10 report ./tests/unit/tests -instr-profile=/tmp/mozillavpn.llvm-final src

printn Y "(unit) Generating the HTML report... "
llvm-cov-10 show ./tests/unit/tests -instr-profile=/tmp/mozillavpn.llvm-final src -format=html > $REPORT_FILE_UNIT || die "Failed to generate the HTML report"
print G $REPORT_FILE_UNIT

if [ -f ./tests/auth/tests ]; then
  export LLVM_PROFILE_FILE=/tmp/mozillavpn_auth.llvm

  print Y "Running the auth tests..."
  ./tests/auth/tests || die "Failed to run tests"
  print G "done."

  unset LLVM_PROFILE_FILE

  printn Y "(auth) Merge the profile data... "
  llvm-profdata-10 merge /tmp/mozillavpn_auth.llvm -o /tmp/mozillavpn.llvm-final || die "Failed to merge the coverage report"
  print G "done."

  print Y "(auth) Report:"
  llvm-cov-10 report ./tests/auth/tests -instr-profile=/tmp/mozillavpn.llvm-final src

  printn Y "(auth) Generating the HTML report... "
  llvm-cov-10 show ./tests/auth/tests -instr-profile=/tmp/mozillavpn.llvm-final src -format=html > $REPORT_FILE_AUTH || die "Failed to generate the HTML report"
  print G $REPORT_FILE_AUTH
fi
