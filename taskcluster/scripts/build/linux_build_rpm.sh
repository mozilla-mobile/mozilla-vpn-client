#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

DIST=$(rpm --eval "%{dist}" | cut -d. -f2-)

helpFunction() {
  echo "Usage: $0 [options]"
  echo ""
  echo "Build options:"
  echo "  -d, --dist DIST     Build packages for distribution DIST (defaut: ${DIST})"
  echo "  -s, --static        Build packages for statically linked Qt."
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

    -s|--static)
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

# Append the build suffix to the package revision.
sed -e "s/^Release: \(.*\)$/Release: \1.${DIST}/" ${MOZ_FETCHES_DIR}/mozillavpn.spec > ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec

# For static Qt, strip out the Qt build and runtime dependencies.
if [[ "$DIST" == "static" ]]; then
  export PATH=${MOZ_FETCHES_DIR}/qt-linux/bin:${PATH}
  sed -rie '/Requires:\s+qt6-/d' ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec
  sed -rie '/BuildRequires:\s+qt6-/d' ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec
fi

# Install the build dependencies.
sudo yum-builddep -y ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec

# Build the packages.
rpmbuild -D "_topdir ${HOME}" -D "_sourcedir ${MOZ_FETCHES_DIR}" -ba ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec

# Gather the build artifacts for export
RPM_BUILD_ARCH=$(uname -m)
tar -C ${HOME}/RPMS/${RPM_BUILD_ARCH} -cvzf /builds/worker/artifacts/mozillavpn-${DIST}.tar.gz .
