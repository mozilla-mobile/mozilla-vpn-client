#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
cd ${TASK_WORKDIR}

# Collect the Cargo package dependencies
mkdir -p cargo-deps/.cargo
cargo vendor --manifest-path ${VCS_PATH}/Cargo.toml | tee cargo-deps/.cargo/config.toml
mv vendor cargo-deps

echo "Build Qt- Creating dist artifact"
mkdir -p ${TASK_WORKDIR}/public/build
tar -cJf ${TASK_WORKDIR}/public/build/cargo-deps.tar.xz cargo-deps/
