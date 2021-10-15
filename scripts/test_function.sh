#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh
LANGUAGE=en
LANG=en

print N "This script runs the functional tests"
print N ""

runTest() {
  print Y "Running the test: $1"
  mocha --file ./tests/functional/setup-vpn.js --bail $1
}

if ! [ -d "src" ] || ! [ -d "tests" ]; then
  die "This script must be executed at the root of the repository."
fi

if [ $# -ne 0 ]; then
  for i in $*; do
    runTest "$i"
  done
else
  for i in tests/functional/test*; do
    runTest "$i"
  done
fi

