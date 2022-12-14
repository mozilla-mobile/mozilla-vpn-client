#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if [ -f .env ]; then
  . .env
fi

. ./scripts/utils/commons.sh

cmake -S . -B ./.vscode/build_osx -GNinja \
    -DCMAKE_PREFIX_PATH=${QT_MACOS_BIN} \
    -DSENTRY_DSN=$SENTRY_DSN \
    -DSENTRY_ENVELOPE_ENDPOINT=$SENTRY_ENVELOPE_ENDPOINT \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_BUILD_TYPE=Debug \


./scripts/utils/generate_glean.py

cmake --build ./.vscode/build_osx -j20 || die

# Sign for own use:
codesign -s "${MACOS_CODESIGN_ID}" .vscode/build_osx/src/Mozilla\ VPN.app 
echo "Signed!"