# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
. $(dirname $0)/../../../scripts/utils/commons.sh

# Ensure all git submodules are checked out
git submodule update --init --recursive

# We need to call bash with a login shell, so that conda is initialized
print Y "Installing conda"
source ${MOZ_FETCHES_DIR}/conda/bin/activate
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
        -DCMAKE_PREFIX_PATH=${MOZ_FETCHES_DIR}/qt-macos/lib/cmake \
        -DQT_HOST_PATH=${MOZ_FETCHES_DIR}/qt-host-tools/ \
        -DQT_HOST_PATH_CMAKE_DIR=${MOZ_FETCHES_DIR}/qt-host-tools/lib/cmake \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
        -DBUILD_TESTS=OFF

print Y "Building the client..."
cmake --build ${TASK_WORKDIR}/build-osxcross

print Y "Compressing the build artifacts..."
mkdir -p ${TASK_WORKDIR}/artifacts/
tar -C ${TASK_WORKDIR}/build-osxcross/src/ -czvf ${TASK_WORKDIR}/artifacts/MozillaVPN.tar.gz "Mozilla VPN.app" || die

print Y "Generating dSYM bundle"
CONTENTS_DIR="${TASK_WORKDIR}/build-osxcross/src/Mozilla VPN.app/Contents"
find "${TASK_WORKDIR}/build-osxcross/src/Mozilla VPN.app/Contents" -type f -executable | while read EXEFILE; do
  if dsymutil -s "${EXEFILE}" | grep N_OSO -q; then
    dsymutil "${EXEFILE}" -o ${TASK_WORKDIR}/MozillaVPN.dSYM
  fi
done
find ${TASK_WORKDIR}/MozillaVPN.dSYM/Contents/Resources/DWARF/ -type f | while read DBGFILE; do
  sentry-cli difutil check "${DBGFILE}"
  sentry-cli difutil bundle-sources "${DBGFILE}"
done
tar -C ${TASK_WORKDIR} -cJvf ${TASK_WORKDIR}/artifacts/MozillaVPN-dsym.tar.xz MozillaVPN.dSYM || die 

print G "Done!"
