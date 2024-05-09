#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

echo "Hello World!"
echo "Dumping the environment..."
env

echo "Dumping /etc/os-release..."
cat /etc/os-release

echo "Dumping installed packages..."
dpkg --get-selections

echo "Docker exists! Let's try it!"
docker run debian:bullseye /bin/echo "Hello World"
