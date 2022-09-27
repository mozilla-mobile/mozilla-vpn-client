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
ls 
cd /builds/worker/fetches/
ls
openssl dgst -sha256 -sign /builds/worker/checkouts/vcs/addons_key.pem -out addons/manifest.json.sign addons/manifest.json

cp -r /builds/worker/fetches/addons /builds/worker/artifacts