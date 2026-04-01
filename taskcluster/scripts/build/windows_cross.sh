# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
. $(dirname $0)/../../../scripts/utils/commons.sh

env

# The arugment, if present, selects the arch to build for
if [[ $# -ge 1 ]]; then
  ARCH=$1
else
  ARCH=x86_64
fi

# Ensure all git submodules are checked out
git submodule update --init --recursive

# We need to call bash with a login shell, so that conda is initialized
print Y "Installing conda"
source ${MOZ_FETCHES_DIR}/conda/bin/activate
conda-unpack

# Use vendored crates - if available.
# HACK: Disabled for aarch64 until https://github.com/briansmith/ring/pull/2216
# can be merged onto mainline. This ensures that we can patch out the ring crate
# without running afoul of vendored crates.
if [[ -d ${MOZ_FETCHES_DIR}/cargo-vendor && "${ARCH}" != "aarch64" ]]; then
mkdir -p .cargo
cat << EOF > .cargo/config.toml
[source.vendored-sources]
directory = "${MOZ_FETCHES_DIR}/cargo-vendor"

[source.crates-io]
replace-with = "vendored-sources"
EOF
fi

print Y "Configuring the build..."
mkdir ${TASK_WORKDIR}/build-win

cmake -S ${VCS_PATH} -B ${TASK_WORKDIR}/build-win -GNinja \
        -DCMAKE_TOOLCHAIN_FILE=${VCS_PATH}/scripts/windows/${ARCH}-toolchain.cmake \
        -DCMAKE_PREFIX_PATH=${MOZ_FETCHES_DIR}/qt-windows/lib/cmake \
        -DQT_HOST_PATH=${MOZ_FETCHES_DIR}/qt-host-tools/ \
        -DQT_HOST_PATH_CMAKE_DIR=${MOZ_FETCHES_DIR}/qt-host-tools/lib/cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=OFF

print Y "Building the client..."
cmake --build ${TASK_WORKDIR}/build-win

print Y "Compressing the build artifacts..."
cmake --install ${TASK_WORKDIR}/build-win --prefix ${TASK_WORKDIR}/unsigned
mkdir -p ${TASK_WORKDIR}/artifacts/
(cd ${TASK_WORKDIR}/unsigned && zip -r ${TASK_WORKDIR}/artifacts/unsigned.zip .)

print G "Done!"
