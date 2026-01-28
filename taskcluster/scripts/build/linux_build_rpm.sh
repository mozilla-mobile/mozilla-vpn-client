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

# Determine the build suffix to generate from /etc/os-release
case ${ID} in
  fedora)
    BUILDSUFFIX="fc${VERSION_ID}"
    ;;

  static)
    BUILDSUFFIX="static"
    ;;

  *)
    echo "Unsupported RPM distribution: ${ID}"
    exit 1
    ;;
esac

# Append the build suffix to the package revision.
sed -e "s/^Release: \(.*\)$/Release: \1.${BUILDSUFFIX}/" ${MOZ_FETCHES_DIR}/mozillavpn.spec > ${MOZ_FETCHES_DIR}/mozillavpn-${BUILDSUFFIX}.spec

# For static Qt, strip out the Qt build and runtime dependencies.
if [[ "$STATICQT" == "Y" ]]; then
  export PATH=${MOZ_FETCHES_DIR}/qt-linux/bin:${PATH}
  sed -rie '/Requires:\s+qt6-/d' ${MOZ_FETCHES_DIR}/mozillavpn-${BUILDSUFFIX}.spec
  sed -rie '/BuildRequires:\s+qt6-/d' ${MOZ_FETCHES_DIR}/mozillavpn-${BUILDSUFFIX}.spec
fi

# Install the build dependencies.
sudo yum-builddep -y ${MOZ_FETCHES_DIR}/mozillavpn-${BUILDSUFFIX}.spec

# Build the packages.
rpmbuild -D "_topdir ${HOME}" -D "_sourcedir ${MOZ_FETCHES_DIR}" -ba ${MOZ_FETCHES_DIR}/mozillavpn-${BUILDSUFFIX}.spec

# Gather the build artifacts for export
RPM_BUILD_ARCH=$(uname -m)
tar -C ${HOME}/RPMS/${RPM_BUILD_ARCH} -cvzf /builds/worker/artifacts/mozillavpn-${ID}-${BUILDSUFFIX}.tar.gz .
