# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


/opt/emsdk/emsdk activate latest
source /opt/emsdk/emsdk/emsdk_env.sh

# This script is used in the Android Debug (universal) build task
git submodule init
git submodule update


pip3 install -r requirements.txt
# glean
python3 ./scripts/utils/generate_glean.py
# translations
python3 ./scripts/utils/import_languages.py

./scripts/wasm/compile.sh
# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/

cp wasm/* /builds/worker/artifacts/

ccache -s
