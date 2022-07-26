#!/bin/bash
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

git submodule sync --recursive
git submodule update --init --force --recursive --depth=1
# Reqs
pip3 install -r requirements.txt

# Get Secrets for building
echo "Fetching Tokens!"
./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k addons_key -f addons_key.pem

openssl dgst -sha256 -sign addons_key.pem -out $MOZ_FETCHES/addons/manifest.json.sign $MOZ_FETCHES/addons/manifest.json

cp -r $MOZ_FETCHES/addons /builds/worker/artifacts