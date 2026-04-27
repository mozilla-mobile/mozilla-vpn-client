#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

## Get the default distribution to build from /etc/os-release
source /etc/os-release

helpFunction() {
  echo "Usage: $0 [options]"
  echo ""
  echo "Build options:"
  echo "  -d, --dist DIST         Build packages for distribution DIST"
  echo "  -s, --static            Build with statically linked Qt (x86_64)"
  echo "  --cross-arch ARCH       Cross-compile for ARCH (supported: aarch64)"
  echo "  -h, --help              Display this message and exit"
}

CROSS_ARCH=""
CROSS_DEB_ARCH=""
STATICQT=N
DIST=""

## Parse arguments
while [[ $# -gt 0 ]]; do
  key="$1"  
  case "$key" in
    -d|--dist)
      DIST="$2"
      shift 2
      ;;
    -s|--static)
      STATICQT=Y
      DIST="static"
      shift
      ;;
    --cross-arch)
      CROSS_ARCH="$2"
      shift 2
      ;;
    -h|--help)
      helpFunction
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      helpFunction
      exit 1
      ;;
  esac
done

# Validate cross-arch and resolve the Debian architecture name
if [[ -n "$CROSS_ARCH" ]]; then
  case "$CROSS_ARCH" in
    aarch64) CROSS_DEB_ARCH="arm64" ;;
    *) echo "ERROR: unsupported --cross-arch value: ${CROSS_ARCH}" >&2; exit 1 ;;
  esac
  STATICQT=Y
  DIST="${DIST:-static-${CROSS_DEB_ARCH}}"
fi

# Fall back to the host operating system if no distribution was specified
DIST="${DIST:-${VERSION_CODENAME}}"

# Register the cross-arch before updating the package database
if [[ -n "$CROSS_ARCH" ]]; then
  sudo dpkg --add-architecture "${CROSS_DEB_ARCH}"
fi

# Update the package database, just in case.
sudo apt-get update

# Find and extract the package source
DSCFILE=$(find "${MOZ_FETCHES_DIR}" -name '*.dsc')
if [[ -f "${DSCFILE}" ]]; then
  dpkg-source -x "${DSCFILE}" "$(pwd)/mozillavpn-source/"
else
  echo "ERROR: Unable to locate DSC file" >&2
  echo "${MOZ_FETCHES_DIR} contained:" >&2
  ls -al "${MOZ_FETCHES_DIR}"
  exit 1
fi

DPKG_PACKAGE_SRCNAME=$(dpkg-parsechangelog -l mozillavpn-source/debian/changelog -S Source)
DPKG_PACKAGE_BASE_VERSION=$(dpkg-parsechangelog -l mozillavpn-source/debian/changelog -S Version)
DPKG_PACKAGE_DIST_VERSION="${DPKG_PACKAGE_BASE_VERSION}-${DIST}1"
DPKG_PACKAGE_BUILD_ARGS="--unsigned-source --build=full"

# Update the changelog entry for this distribution
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

# For static Qt or cross builds, strip out the Qt build and runtime dependencies.
if [[ "$STATICQT" == "Y" ]]; then
  sed -rie '/\s+(qt6-|qml6-|libqt6|qmake)/d' "$(pwd)/mozillavpn-source/debian/control"
fi

# Set up cross-compilation environment
if [[ -n "$CROSS_ARCH" ]]; then
  MK_BUILD_DEPS_ARGS="--host-arch ${CROSS_DEB_ARCH}"
  DPKG_PACKAGE_BUILD_ARGS="-d --unsigned-source --build=binary ${MK_BUILD_DEPS_ARGS}"
  export QT_AARCH64_PATH="${MOZ_FETCHES_DIR}/qt-linux"
  export QT_HOST_PATH="${MOZ_FETCHES_DIR}/qt-host-tools"

  if [[ ! -d "${QT_AARCH64_PATH}" ]]; then
    echo "qt-linux-${CROSS_ARCH} not found at ${QT_AARCH64_PATH}" >&2
    exit 1
  fi
  if [[ ! -d "${QT_HOST_PATH}" ]]; then
    echo "qt-host-tools not found at ${QT_HOST_PATH}" >&2
    exit 1
  fi

  # Put Qt host tools on PATH so cross-build CMake steps can find moc/rcc
  export PATH="${QT_HOST_PATH}/bin:${PATH}"

  # Rust cross-compilation
  export CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER="${CROSS_ARCH}-linux-gnu-gcc"
  export CARGO_BUILD_TARGET="${CROSS_ARCH}-unknown-linux-gnu"
  if command -v rustup > /dev/null 2>&1; then
    rustup target add "${CROSS_ARCH}-unknown-linux-gnu" 2>/dev/null || true
  fi

  # Go cross-compilation
  export GOARCH="${CROSS_DEB_ARCH}"
  export GOOS=linux
  export CGO_ENABLED=1
  export CC="${CROSS_ARCH}-linux-gnu-gcc"
  export CXX="${CROSS_ARCH}-linux-gnu-g++"
elif [[ "$STATICQT" == "Y" ]]; then
  export PATH="${MOZ_FETCHES_DIR}/qt-linux/bin:${PATH}"
fi

# Install build dependencies
mk-build-deps $MK_BUILD_DEPS_ARGS "$(pwd)/mozillavpn-source/debian/control"
sudo apt -y install "./${DPKG_PACKAGE_SRCNAME}-build-deps_${DPKG_PACKAGE_DIST_VERSION}_all.deb"
rm -f "./${DPKG_PACKAGE_SRCNAME}-build-deps_${DPKG_PACKAGE_DIST_VERSION}_*"

(cd mozillavpn-source/ && dpkg-buildpackage $DPKG_PACKAGE_BUILD_ARGS)

# Compress all build artifacts into a tarball
BUILD_ARTIFACTS=$(find . -maxdepth 1 -type f -name 'mozillavpn*' -printf '%f\n')
tar -cvzf /builds/worker/artifacts/mozillavpn-${DIST}.tar.gz ${BUILD_ARTIFACTS}

# Copy individual build artifacts for upload
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
