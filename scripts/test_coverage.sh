#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

REPORT_FILE=/tmp/report.html
LANGUAGE=en
LANG=en

print N "This script runs the unit tests and shows the test coverage."
print N ""

if ! [ -d "src" ] || ! [ -d "tests" ]; then
  die "This script must be executed at the root of the repository."
fi

print Y "Compiling..."
make || die "Failed to compile"
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

print Y "Running the auth tests..."
./tests/auth/tests || die "Failed to run tests"
print G "done."

printn Y "Merge the profile data... "
llvm-profdata-10 merge /tmp/mozillavpn.llvm -o /tmp/mozillavpn.llvm-final || die "Failed to merge the coverage report"
print G "done."

print Y "Report:"
llvm-cov-10 report ./tests/unit/tests -instr-profile=/tmp/mozillavpn.llvm-final src

printn Y "Generating the HTML report... "
llvm-cov-10 show ./tests/unit/tests -instr-profile=/tmp/mozillavpn.llvm-final src -format=html > $REPORT_FILE || die "Failed to generate the HTML report"
print G $REPORT_FILE
