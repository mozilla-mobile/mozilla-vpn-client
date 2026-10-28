#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

DIST=$(rpm --eval "%{dist}" | cut -d. -f2-)
CROSS_ARCH=""

helpFunction() {
  echo "Usage: $0 [options]"
  echo ""
  echo "Build options:"
  echo "  -d, --dist DIST     Build packages for distribution DIST (defaut: ${DIST})"
  echo "  -s, --static        Build packages for statically linked Qt."
  echo "  --cross-arch ARCH   Cross-compile for ARCH from a Debian host (supported: aarch64)"
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

    --cross-arch)
      CROSS_ARCH="$2"
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

# Validate the cross-compilation architecture
if [[ -n "$CROSS_ARCH" ]]; then
  case "$CROSS_ARCH" in
    aarch64) CROSS_TRIPLE="aarch64-linux-gnu" ;;
    *) echo "ERROR: unsupported --cross-arch value: ${CROSS_ARCH}" >&2; exit 1 ;;
  esac
  DIST="static"
fi

# Append the build suffix to the package revision.
sed -e "s/^Release: \(.*\)$/Release: \1.${DIST}/" ${MOZ_FETCHES_DIR}/mozillavpn.spec > ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec

# For static Qt, strip out the Qt build and runtime dependencies.
if [[ "$DIST" == "static" ]]; then
  sed -rie '/Requires:\s+qt6-/d' ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec
  sed -rie '/BuildRequires:\s+qt6-/d' ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec
fi

RPMBUILD_ARGS=(-D "_topdir ${HOME}" -D "_sourcedir ${MOZ_FETCHES_DIR}")
if [[ -n "$CROSS_ARCH" ]]; then
  # Cross-compiling from a Debian host: the build dependencies are provided by
  # the docker image rather than an RPM database, and the RPM macros for CMake
  # and systemd are not available, so we provide our own.
  QT_CROSS_PATH="${MOZ_FETCHES_DIR}/qt-linux"
  QT_HOST_PATH="${MOZ_FETCHES_DIR}/qt-host-tools"
  for QT_DIR in "${QT_CROSS_PATH}" "${QT_HOST_PATH}"; do
    if [[ ! -d "${QT_DIR}" ]]; then
      echo "ERROR: Qt not found at ${QT_DIR}" >&2
      exit 1
    fi
  done
  export PATH=${QT_HOST_PATH}/bin:${PATH}

  CMAKE_CROSS_ARGS=(
    -G Ninja
    -DCMAKE_BUILD_TYPE=RelWithDebInfo
    -DCMAKE_INSTALL_PREFIX=/usr
    -DCMAKE_TOOLCHAIN_FILE='${PWD}'/scripts/linux/${CROSS_ARCH}-toolchain.cmake
    -DCMAKE_FIND_ROOT_PATH=/
    -DCMAKE_LIBRARY_ARCHITECTURE=${CROSS_TRIPLE}
    -DCMAKE_PREFIX_PATH=${QT_CROSS_PATH}
    -DQT_HOST_PATH=${QT_HOST_PATH}
    -DQT_HOST_PATH_CMAKE_DIR=${QT_HOST_PATH}/lib/cmake
    -DBUILD_TESTS=OFF
    -DBUILD_APPARMOR=OFF
    -DLINUX_POLKIT_RULES=others
    -DSYSTEMD_UNIT_DIR=/usr/lib/systemd/system
  )

  RPMBUILD_ARGS+=(
    --target ${CROSS_ARCH}
    --nodeps
    -D "_buildshell /bin/bash"
    -D "_unitdir /usr/lib/systemd/system"
    -D "debug_package %{nil}"
    -D "_build_id_links none"
    -D "__strip /usr/bin/${CROSS_TRIPLE}-strip"
    -D "__objdump /usr/bin/${CROSS_TRIPLE}-objdump"
    -D "cmake cmake -S . -B build-${CROSS_ARCH} ${CMAKE_CROSS_ARGS[*]}"
    -D "cmake_build cmake --build build-${CROSS_ARCH}"
    -D "cmake_install DESTDIR=%{buildroot} cmake --install build-${CROSS_ARCH}"
  )
  RPM_BUILD_ARCH=${CROSS_ARCH}
  PACKAGE_SUFFIX="_${CROSS_ARCH}"
else
  if [[ "$DIST" == "static" ]]; then
    export PATH=${MOZ_FETCHES_DIR}/qt-linux/bin:${PATH}
  fi

  # Install the build dependencies.
  sudo yum-builddep -y ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec
  RPM_BUILD_ARCH=$(rpm --eval '%{_arch}')
  PACKAGE_SUFFIX=""
fi

# Build the packages.
rpmbuild "${RPMBUILD_ARGS[@]}" -ba ${MOZ_FETCHES_DIR}/mozillavpn-${DIST}.spec

# Gather the build artifacts for export
tar -C ${HOME}/RPMS/${RPM_BUILD_ARCH} -cvzf /builds/worker/artifacts/mozillavpn-${DIST}${PACKAGE_SUFFIX}.tar.gz .
find ${HOME}/RPMS/${RPM_BUILD_ARCH} -name 'mozillavpn-*.rpm' | while read -r RPM_BUILD_FILE; do
  RPM_BUILD_PACKAGE=$(basename ${RPM_BUILD_FILE} | grep -E -o '^mozillavpn(-[a-z]+)?')
  cp -v ${RPM_BUILD_FILE} /builds/worker/artifacts/${RPM_BUILD_PACKAGE}${PACKAGE_SUFFIX}.rpm
done
