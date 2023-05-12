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

mkdir build-addons
cmake -S $(pwd)/addons -B $TASK_WORKDIR/addons -GNinja
cmake --build $TASK_WORKDIR/addons

zip -r /builds/worker/artifacts/addons.zip $TASK_WORKDIR/addons -i '*.rcc' $TASK_WORKDIR/addons/manifest.json
cp $TASK_WORKDIR/addons/*.rcc $TASK_WORKDIR/artifacts
cp $TASK_WORKDIR/addons/manifest.json $TASK_WORKDIR/artifacts
