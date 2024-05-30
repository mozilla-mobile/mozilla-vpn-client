#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
cd ${TASK_WORKDIR}

# Collect the Cargo package dependencies
echo "Downloading Cargo Dependencies"
mkdir -p cargo-deps/
cargo vendor --manifest-path ${VCS_PATH}/Cargo.toml cargo-deps/

echo "Compressing Cargo Dependencies"
tar -cJf ${UPLOAD_DIR}/cargo-deps.tar.xz cargo-deps/
