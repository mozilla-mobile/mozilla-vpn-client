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
  echo "  -d, --dist DIST     Build packages for distribution DIST (defaut: ${VERSION_CODENAME})"
  echo "  -s, --static        Build packages for statically linked Qt."
  echo "  -h, --help          Display this message and exit"
}

STATICQT=N

## Parse arguments
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -d|--dist)
      DIST="$2"
      shift
      shift
      ;;
    
    -s|--static)
      STATICQT=Y
      DIST="static"
      shift
      ;;

    -h|--help)
      helpFunction
      shift
      exit 0
      ;;

    *)
      echo "Unknown argument: $1" >&2
      helpFunction
      shift
      exit 1
      ;;
  esac
done

# Fall back to the host operating system if no distribution was specified
if [[ -z "$DIST" ]]; then
  DIST="${VERSION_CODENAME}"
fi

# Update the package database, just in case.
sudo apt-get update

# Find and extract the package source
DSCFILE=$(find ${MOZ_FETCHES_DIR} -name '*.dsc')
if [[ ! -f "$DSCFILE" ]]; then
    echo "Unable to locate DSC file" >&2
    echo "${MOZ_FETCHES_DIR} contained:" >2&
    ls -al ${MOZ_FETCHES_DIR}
    exit 1
fi
dpkg-source -x ${DSCFILE} $(pwd)/mozillavpn-source/
DPKG_PACKAGE_SRCNAME=$(dpkg-parsechangelog -l mozillavpn-source/debian/changelog -S Source)
DPKG_PACKAGE_BASE_VERSION=$(dpkg-parsechangelog -l mozillavpn-source/debian/changelog -S Version)
DPKG_PACKAGE_DIST_VERSION=${DPKG_PACKAGE_BASE_VERSION}-${DIST}1
DPKG_PACKAGE_BUILD_ARGS="--unsigned-source"

# Update the changelog to release for the target distribution.
if [[ -z "$TASK_OWNER" ]]; then
  export DEBEMAIL="vpn@mozilla.com"
  export DEBFULLNAME="Mozilla VPN Team"
else
  export DEBEMAIL="${TASK_OWNER}"
  export DEBFULLNAME=$(echo ${TASK_OWNER} | cut -d@ -f1)
fi
dch -c $(pwd)/mozillavpn-source/debian/changelog -v ${DPKG_PACKAGE_DIST_VERSION} \
    -D ${DIST} --force-distribution "Release for ${DIST}"

# For static Qt, strip out the Qt build and runtime dependencies.
if [[ "$STATICQT" == "Y" ]]; then
  export PATH=${MOZ_FETCHES_DIR}/qt-linux/bin:${PATH}
  sed -rie '/\s+(qt6-|qml6-|libqt6|qmake)/d' $(pwd)/mozillavpn-source/debian/control
  # Remove the golang and cmake dependencies if manually installed
  if which cmake >/dev/null 2>&1; then
    sed -rie '/\s+(cmake)/d' $(pwd)/mozillavpn-source/debian/control
  fi
  if which go >/dev/null 2>&1; then
    sed -rie '/\s+(golang)/d' $(pwd)/mozillavpn-source/debian/control
  fi
fi

# Install the package build dependencies.
mk-build-deps $(pwd)/mozillavpn-source/debian/control
sudo apt -y install ./${DPKG_PACKAGE_SRCNAME}-build-deps_${DPKG_PACKAGE_DIST_VERSION}_all.deb
rm -f ./${DPKG_PACKAGE_SRCNAME}-build-deps_${DPKG_PACKAGE_DIST_VERSION}_*

# Build the packages
(cd mozillavpn-source/ && dpkg-buildpackage --unsigned-source --build=full)

# Compress all build artifacts into a tarball
BUILD_ARTIFACTS=$(find . -maxdepth 1 -type f -name 'mozillavpn*' -printf '%f\n')
tar -cvzf /builds/worker/artifacts/mozillavpn-${DIST}.tar.gz ${BUILD_ARTIFACTS}

# Copy individual build artifacts for upload
for FILENAME in ${BUILD_ARTIFACTS}; do
  PACKAGE_NAME=$(echo "${FILENAME}" | cut -d_ -f1)
  PACKAGE_EXT=$(echo "${FILENAME}" | grep -o '[^.]*$')
  if [ "$(echo ${FILENAME} | cut -d_ -f2)" != "${DPKG_PACKAGE_DIST_VERSION}" ]; then
    continue
  fi
  if echo "${PACKAGE_NAME}" | grep -e "-dbgsym$"; then
    PACKAGE_EXT="ddeb"
  fi
  cp -v ${FILENAME} /builds/worker/artifacts/${PACKAGE_NAME}.${PACKAGE_EXT}
done
