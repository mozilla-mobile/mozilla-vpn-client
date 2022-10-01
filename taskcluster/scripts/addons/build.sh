#!/bin/bash
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

git submodule sync --recursive
git submodule update --init --force --recursive --depth=1


# Reqs
pip3 install -r requirements.txt

python3 scripts/addon/generate_all.py
cd addons/generated/

zip -r /builds/worker/artifacts/addons.zip addons
cp addons/*.rcc $TASK_WORKDIR/artifacts
cp addons/manifest.json $TASK_WORKDIR/artifacts
