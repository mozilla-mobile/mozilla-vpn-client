# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
. $(dirname $0)/../../../scripts/utils/commons.sh

# Ensure all git submodules are checked out
git submodule update --init --depth 1

# We need to call bash with a login shell, so that conda is initialized
print Y "Installing conda"
source ${MOZ_FETCHES_DIR}/conda/bin/activate
conda-unpack

print Y "Configuring the build..."
mkdir ${TASK_WORKDIR}/build-wasm
${MOZ_FETCHES_DIR}/qt-wasm/bin/qt-cmake -S . -B ${TASK_WORKDIR}/build-wasm -GNinja \
    -DQT_HOST_PATH=${MOZ_FETCHES_DIR}/qt-host-tools/ \
    -DQT_HOST_PATH_CMAKE_DIR=${MOZ_FETCHES_DIR}/qt-host-tools/lib/cmake \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DBUILD_TESTS=OFF

print Y "Building the client..."
cmake --build ${TASK_WORKDIR}/build-wasm

print Y "Compressing the build artifacts..."
mkdir -p ${TASK_WORKDIR}/artifacts/
cp -r ${TASK_WORKDIR}/build-wasm/wasm_build/* ${TASK_WORKDIR}/artifacts/

print G "Done!"
