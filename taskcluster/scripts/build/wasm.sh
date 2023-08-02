# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

source /opt/emsdk/emsdk_env.sh

# Reqs
git submodule update --init --depth 1

pip3 install -r requirements.txt

export PATH="$QTPATH/wasm_32/bin:$PATH"
mkdir build
$QTPATH/wasm_32/bin/qt-cmake -S . -B build -DQT_HOST_PATH=/$QTPATH/gcc_64 -DQT_HOST_PATH_CMAKE_DIR=/$QTPATH/gcc_64/lib/cmake
cmake --build build -j8

# Artifacts should be placed here!
mkdir -p /builds/worker/artifacts/

cp -r build/wasm_build/* /builds/worker/artifacts/

ccache -s
