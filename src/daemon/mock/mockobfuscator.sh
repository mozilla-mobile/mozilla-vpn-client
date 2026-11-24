#!/usr/bin/env bash
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
echo "Mock obfuscator arguments: $@" >&2

# Parse out the listening port argument.
LISTEN=0
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -r | --listen-port)
    LISTEN="$2"
    shift
    shift
    ;;
  *)
    shift
    ;;
  esac
done

# Choose a random port if none were specified
if [ $LISTEN -eq 0 ]; then
    LISTEN=$(($RANDOM % 16384 + 49152))
fi

echo "listening on 127.0.0.1:${LISTEN}"
while sleep 60; do
    true
done
