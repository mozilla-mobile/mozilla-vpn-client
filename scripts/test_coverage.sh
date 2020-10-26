#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

export LLVM_PROFILE_FILE=/tmp/mozillavpn.llvm-report
REPORT_FILE=/tmp/report.html

print N "This script runs tests and show the coverage"
print N ""

if ! [ -d "src" ] && ! [ -d "tests" ]; then
  die "This script must be executed at the root of the repository."
fi

print Y "Compiling..."
make || die "Failed to compile"
print G "done."

print Y "Running tests..."
./tests/testModels || die "Failed to run tests"
print G "done."

if ! [ -f $LLVM_PROFILE_FILE ]; then
  die "No report generated!"
fi

printn Y "Merge the coverage report... "
llvm-profdata-10 merge $LLVM_PROFILE_FILE -o $LLVM_PROFILE_DATA.final || die "Failed to merge the coverage report"
print G "done."

print Y "Report:"
llvm-cov-10 report ./tests/testModels -instr-profile=$LLVM_PROFILE_DATA.final src

printn Y "Generating the HTML report... "
llvm-cov-10 show ./tests/testModels -instr-profile=$LLVM_PROFILE_DATA.final src -format=html > $REPORT_FILE || die "Failed to generate the HTML report"
print G $REPORT_FILE
