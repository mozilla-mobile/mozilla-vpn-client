#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

print N "This script runs functiona tests"
print N ""

# TODO: support the execution for macos and windows
if ! [ -f "src/mozillavpn" ] || ! [ -d "tests" ]; then
  die "This script must be executed at the root of the repository."
fi

for i in tests/functional/test*; do
  print Y "Running the app..."
  ./src/mozillavpn &>/dev/null &
  PID=$!
  print G "done."

  print Y "Running the test: $i"
  mocha $i || ERROR=yes

  wait

  if [ "$ERROR" = yes ]; then
    print R "Nooo"
    exit 1
  fi
done
