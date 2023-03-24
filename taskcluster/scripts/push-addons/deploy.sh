#!/bin/bash
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

# Reqs
git submodule update --init --depth 1

for i in src/apps/*/translations/i18n; do
  git submodule update --remote $i
done

pip3 install -r requirements.txt

# Get Secrets for building
echo "Fetching Tokens!"
./taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k addons_key -f addons_key.pem
ls
cd /builds/worker/fetches/
ls
openssl dgst -sha256 -sign /builds/worker/checkouts/vcs/addons_key.pem -out addons/manifest.json.sig addons/manifest.json

cp -r /builds/worker/fetches/addons /builds/worker/artifacts
