#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# Install the mozillavpn.deb package.
apt-get update
apt install $MOZ_FETCHES_DIR/mozillavpn.deb

echo "Installed version"
mozillavpn --version
