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
  echo "  -h, --help          Display this message and exit"
}

## Parse arguments
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -d|--dist)
      DIST="$2"
      shift
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

# Determine the build suffix to generate from /etc/os-release
case ${ID} in
  fedora)
    BUILDSUFFIX="fc${VERSION_ID}"
    ;;

  *)
    echo "Unsupported RPM distribution: ${ID}"
    exit 1
    ;;
esac

RPM_BUILD_ARCH=$(uname -m)

# Install the build dependencies.
sudo yum-builddep -y ${MOZ_FETCHES_DIR}/mozillavpn.spec

# Append the build suffix to the package revision.
sed -e "s/^Release: \(.*\)$/Release: \1.${BUILDSUFFIX}/" ${MOZ_FETCHES_DIR}/mozillavpn.spec > ${MOZ_FETCHES_DIR}/mozillavpn-${BUILDSUFFIX}.spec

# Build the packages.
rpmbuild -D "_topdir ${HOME}" -D "_sourcedir ${MOZ_FETCHES_DIR}" -ba ${MOZ_FETCHES_DIR}/mozillavpn-${BUILDSUFFIX}.spec

# Gather the build artifacts for export
tar -C ${HOME}/RPMS/${RPM_BUILD_ARCH} -cvzf /builds/worker/artifacts/mozillavpn-${ID}-${BUILDSUFFIX}.tar.gz .

# Check for unintended writes to the source directory.
echo "Ensuring the source dir is clean:"
$(pwd)/mozillavpn-source/scripts/utils/dirtycheck.sh
