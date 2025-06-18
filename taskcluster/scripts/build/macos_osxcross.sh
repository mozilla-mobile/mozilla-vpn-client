# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
. $(dirname $0)/../../../scripts/utils/commons.sh

# We need to call bash with a login shell, so that conda is intitialized
print Y "Installing conda"
source ${MOZ_FETCHES_DIR}/bin/activate
conda-unpack

# Use vendored crates - if available.
if [ -d ${MOZ_FETCHES_DIR}/cargo-vendor ]; then
mkdir -p .cargo
cat << EOF > .cargo/config.toml
[source.vendored-sources]
directory = "${MOZ_FETCHES_DIR}/cargo-vendor"

[source.crates-io]
replace-with = "vendored-sources"
EOF
fi

print Y "Configuring the build..."
mkdir ${TASK_WORKDIR}/build-osxcross

cmake -S . -B ${TASK_WORKDIR}/build-osxcross -GNinja \
        -DCMAKE_TOOLCHAIN_FILE=scripts/macos/osxcross-toolchain.cmake \
        -DCMAKE_PREFIX_PATH=${MOZ_FETCHES_DIR}/qt_dist/lib/cmake \
        -DQT_HOST_PATH=${CONDA_PREFIX} \
        -DQT_HOST_PATH_CMAKE_DIR=${CONDA_PREFIX}/lib/cmake \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
        -DBUILD_TESTS=OFF

print Y "Inspecting config.toml..."
cat ${TASK_WORKDIR}/build-osxcross/cargo_home/config.toml
print Y "Inspecting env..."
env

print Y "Building the client..."
cmake --build ${TASK_WORKDIR}/build-osxcross
