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

echo "User is: $USER (uid: $UID)"

## Wait, are we building as root? That seems wrong.
echo "Listing env:"
env

# Fall back to the host operating system if no distribution was specified
if [[ -z "$DIST" ]]; then
  DIST="${VERSION_CODENAME}"
fi

# Install a PPA for Qt packages, if necessary.
if [[ -n "$QTPPA" ]]; then
    sudo add-apt-repository -y ${QTPPA}
fi

echo "Listing ${MOZ_FETCHES_DIR}:"
ls -al ${MOZ_FETCHES_DIR}

# Install the build dependencies.
sudo yum-builddep -y ${MOZ_FETCHES_DIR}/mozillavpn.spec

# Build the packages.
rpmbuild -D "_topdir ${MOZ_FETCHES_DIR}" -D "_sourcedir ${MOZ_FETCHES_DIR}" -ba ${MOZ_FETCHES_DIR}/mozillavpn.spec

# Let's see what got built...
echo "Listing $(pwd):"
ls -al $(pwd)

echo "Listing ${MOZ_FETCHES_DIR}:"
ls -al ${MOZ_FETCHES_DIR}
