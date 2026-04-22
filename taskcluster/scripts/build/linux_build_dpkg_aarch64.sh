#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

#
# Inputs (via MOZ_FETCHES_DIR):
#   mozillavpn-sources.tar.gz  - VPN source package (.dsc + sources)
#   qt-linux-aarch64/          - Cross-compiled static Qt6 for aarch64
#   qt-host-tools/             - Qt6 host tools (moc, rcc, etc.) for x86_64

set -e

source /etc/os-release

DIST="static-arm64"

helpFunction() {
  echo "Usage: $0 [options]"
  echo ""
  echo "Build options:"
  echo "  -d, --dist DIST   Build packages for distribution DIST (default: ${DIST})"
  echo "  -h, --help        Display this message and exit"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -d|--dist) DIST="$2"; shift 2 ;;
    -h|--help) helpFunction; exit 0 ;;
    *) echo "Unknown argument: $1" >&2; helpFunction; exit 1 ;;
  esac
done

export QT_AARCH64_PATH="${MOZ_FETCHES_DIR}/qt-linux-aarch64"
export QT_HOST_PATH="${MOZ_FETCHES_DIR}/qt-host-tools"

if [[ ! -d "${QT_AARCH64_PATH}" ]]; then
    echo "ERROR: qt-linux-aarch64 not found at ${QT_AARCH64_PATH}" >&2
    exit 1
fi
if [[ ! -d "${QT_HOST_PATH}" ]]; then
    echo "ERROR: qt-host-tools not found at ${QT_HOST_PATH}" >&2
    exit 1
fi

# Ensure arm64 multiarch is registered
sudo dpkg --add-architecture arm64
sudo apt-get update -q

# Find and extract the source package.
DSCFILE=$(find "${MOZ_FETCHES_DIR}" -name '*.dsc' | head -1)
if [[ -f "${DSCFILE}" ]]; then
    dpkg-source -x "${DSCFILE}" "$(pwd)/mozillavpn-source/"
else
    # Look for a pre-extracted source tree that already contains debian/
    SRCTREE=$(find "${MOZ_FETCHES_DIR}" -maxdepth 3 -name 'debian' -type d \
              2>/dev/null | head -1 | xargs -I{} dirname {})
    if [[ -d "${SRCTREE}" ]]; then
        echo "No .dsc found; using pre-extracted source tree: ${SRCTREE}"
        cp -a "${SRCTREE}" "$(pwd)/mozillavpn-source"
    else
        echo "ERROR: No .dsc file and no source tree with debian/ found under ${MOZ_FETCHES_DIR}" >&2
        find "${MOZ_FETCHES_DIR}" -maxdepth 3 | head -40 >&2
        exit 1
    fi
fi

DPKG_PACKAGE_SRCNAME=$(dpkg-parsechangelog -l mozillavpn-source/debian/changelog -S Source)
DPKG_PACKAGE_BASE_VERSION=$(dpkg-parsechangelog -l mozillavpn-source/debian/changelog -S Version)
DPKG_PACKAGE_DIST_VERSION="${DPKG_PACKAGE_BASE_VERSION}-${DIST}1"

# Update the changelog entry for this distribution.
if [[ -z "${TASK_OWNER}" ]]; then
    export DEBEMAIL="vpn@mozilla.com"
    export DEBFULLNAME="Mozilla VPN Team"
else
    export DEBEMAIL="${TASK_OWNER}"
    export DEBFULLNAME=$(echo "${TASK_OWNER}" | cut -d@ -f1)
fi
dch -c "$(pwd)/mozillavpn-source/debian/changelog" \
    -v "${DPKG_PACKAGE_DIST_VERSION}" \
    -D "${DIST}" --force-distribution "Release for ${DIST}"

# Strip Qt6 package dependencies from debian/control - Qt is statically linked.
sed -rie '/\s+(qt6-|qml6-|libqt6|qmake)/d' "$(pwd)/mozillavpn-source/debian/control"

# Set up Rust cross-compilation for the aarch64-unknown-linux-gnu target.
export CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER=aarch64-linux-gnu-gcc
export CARGO_BUILD_TARGET=aarch64-unknown-linux-gnu

# Ensure the aarch64 Rust target is installed.
if command -v rustup > /dev/null 2>&1; then
    rustup target add aarch64-unknown-linux-gnu 2>/dev/null || true
fi

# Set up Go cross-compilation.
export GOARCH=arm64
export GOOS=linux
export CGO_ENABLED=1
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++

# Put Qt host tools on PATH so that cross-build cmake steps can find moc/rcc.
export PATH="${QT_HOST_PATH}/bin:${PATH}"

# 1. Native build tools (run on the x86_64 host)
sudo apt-get -y install --no-install-recommends \
    cdbs \
    cmake \
    debhelper \
    dh-apparmor \
    flex \
    ninja-build \
    python3-click \
    python3-jinja2 \
    python3-yaml

# 2. arm64 link libraries (linked into the aarch64 target binary).
sudo apt-get -y install --no-install-recommends \
    libcap-dev:arm64 \
    libgcrypt20-dev:arm64 \
    libgl-dev:arm64 \
    libopengl-dev:arm64 \
    libpolkit-gobject-1-dev:arm64 \
    libxkbcommon-dev:arm64

# Build the packages for arm64.
(cd mozillavpn-source/ && \
    dpkg-buildpackage \
        --build=binary \
        --no-sign \
        --host-arch=arm64 \
        -d)

# Compress all build artifacts into a tarball.
BUILD_ARTIFACTS=$(find . -maxdepth 1 -type f -name 'mozillavpn*' -printf '%f\n')
tar -cvzf /builds/worker/artifacts/mozillavpn-${DIST}.tar.gz ${BUILD_ARTIFACTS}

# Copy individual build artifacts for upload.
for FILENAME in ${BUILD_ARTIFACTS}; do
    PACKAGE_NAME=$(echo "${FILENAME}" | cut -d_ -f1)
    PACKAGE_EXT=$(echo "${FILENAME}" | grep -o '[^.]*$')
    if [[ "$(echo ${FILENAME} | cut -d_ -f2)" != "${DPKG_PACKAGE_DIST_VERSION}" ]]; then
        continue
    fi
    if echo "${PACKAGE_NAME}" | grep -q -e "-dbgsym$"; then
        PACKAGE_EXT="ddeb"
    fi
    cp -v "${FILENAME}" "/builds/worker/artifacts/${PACKAGE_NAME}.${PACKAGE_EXT}"
done
