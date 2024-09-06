#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e

cargo vendor --manifest-path $VCS_PATH/Cargo.toml $(pwd)/cargo-vendor
tar -cJf $UPLOAD_DIR/cargo-vendor.tar.xz cargo-vendor/
