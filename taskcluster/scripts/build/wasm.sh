# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


source /opt/emsdk/emsdk_env.sh


echo "TEST"
ls /opt/emsdk
ls /opt/emsdk/emsdk
ls /opt/6.2.3/wasm_32/bin
ls /opt/6.2.3/gcc_64/bin

where qmake 
qmake --version

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

cp -r wasm/* /builds/worker/artifacts/

ccache -s
