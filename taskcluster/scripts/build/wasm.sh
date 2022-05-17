# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

source /opt/emsdk/emsdk_env.sh

which qmake 
qmake --version

git submodule init
git submodule update


pip3 install -r requirements.txt
# glean
python3 ./scripts/utils/generate_glean.py
# translations
python3 ./scripts/utils/import_languages.py

# Add the Wasm qmake after import languages into the path,
# Otherwise import_languages.py will search for lupdate 
# in the wasm folder, but the qt does not seem to ship it in the wasm build. 
export PATH="$QTPATH/wasm_32/bin:$PATH"
./scripts/wasm/compile.sh
# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/

cp -r wasm/* /builds/worker/artifacts/

ccache -s
