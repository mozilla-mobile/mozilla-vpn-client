#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
bash scripts/linux/script.sh --source

cd .tmp
tar -zvcf /builds/worker/artifacts/target.tar.gz .
