#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
cd $HOME
export XZ_DEFAULTS="-T0"

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
QT_SOURCE_VERSION=$(echo $QT_SOURCE_DIR | awk -F"-" '{print $NF}')
ARTIFACT_NAME="qt6_linux.tar.xz"
CROSS_ARCH=""
APT_ARCH=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --cross-arch) CROSS_ARCH="$2"; shift 2 ;;
        *) echo "Unknown argument: $1" >&2; exit 1 ;;
    esac
done

# Validate cross-arch and set apt arch
if [[ -n "$CROSS_ARCH" ]]; then
  case "$CROSS_ARCH" in
    aarch64) APT_ARCH=":arm64" ;;
    *) echo "ERROR: unsupported --cross-arch value: ${CROSS_ARCH}" >&2; exit 1 ;;
  esac
fi

if [[ -n "$CROSS_ARCH" ]]; then
    QT_HOST_TOOLS="${MOZ_FETCHES_DIR}/qt-host-tools"
    if [[ ! -d "${QT_HOST_TOOLS}" ]]; then
        echo "ERROR: qt-host-tools not found at ${QT_HOST_TOOLS}" >&2
        echo "Cross-compilation requires the qt-host-tools toolchain artifact." >&2
        exit 1
    fi
    ARTIFACT_NAME="qt6_linux_${CROSS_ARCH}.tar.xz"
fi

if [[ $(echo -e "${QT_SOURCE_VERSION}\n6.10.3" | sort --version-sort | head -1) != "6.10.3" ]]; then
    echo "Patching for QTBUG-141830"
    patch -d "${QT_SOURCE_DIR}/qtdeclarative" -p1 < "${VCS_PATH}/taskcluster/scripts/toolchain/patches/qtbug-141830-qsortfilterproxymodel.patch"
fi

echo "Patching QWaylandAdwaitaDecorations to add shadows"
patch -d "${QT_SOURCE_DIR}/qtwayland" -p1 < "${VCS_PATH}/taskcluster/scripts/toolchain/patches/VPN-7529-qwaylandadwaitadecoration-add-shadows.patch"


echo "Installing Qt build dependencies"
if [ -f /etc/redhat-release ]; then
    sudo yum -y install \
            gcc-toolset-10 \
            at-spi2-core-devel \
            openssl3-devel
    source /opt/rh/gcc-toolset-10/enable
elif [ -f /etc/debian_version ]; then
    sudo apt-get -y install \
            libatspi2.0-dev${APT_ARCH} \
            libdbus-1-dev${APT_ARCH} \
            libfontconfig1-dev${APT_ARCH} \
            libfreetype6-dev${APT_ARCH} \
            libssl-dev${APT_ARCH} \
            libx11-dev${APT_ARCH} \
            libx11-xcb-dev${APT_ARCH} \
            libxext-dev${APT_ARCH} \
            libxfixes-dev${APT_ARCH} \
            libxi-dev${APT_ARCH} \
            libxrender-dev${APT_ARCH} \
            libxcb1-dev${APT_ARCH} \
            libxcb-cursor-dev${APT_ARCH} \
            libxcb-glx0-dev${APT_ARCH} \
            libxcb-keysyms1-dev${APT_ARCH} \
            libxcb-image0-dev${APT_ARCH} \
            libxcb-shm0-dev${APT_ARCH} \
            libxcb-icccm4-dev${APT_ARCH} \
            libxcb-sync-dev${APT_ARCH} \
            libxcb-xfixes0-dev${APT_ARCH} \
            libxcb-shape0-dev${APT_ARCH} \
            libxcb-randr0-dev${APT_ARCH} \
            libxcb-render-util0-dev${APT_ARCH} \
            libxcb-util-dev${APT_ARCH} \
            libxcb-xinerama0-dev${APT_ARCH} \
            libxcb-xkb-dev${APT_ARCH} \
            libxkbcommon-dev${APT_ARCH} \
            libxkbcommon-x11-dev${APT_ARCH}
fi


echo "Building $(basename $QT_SOURCE_DIR)"
mkdir qt-linux

COMPILE_ARGS=("${QT_SOURCE_DIR}" "$(pwd)/qt-linux" -b "$(pwd)/qt_build")
if [[ -n "$CROSS_ARCH" ]]; then
    COMPILE_ARGS+=(
        --cross-arch "${CROSS_ARCH}"
        --host-path "${QT_HOST_TOOLS}"
        --toolchain-file "${VCS_PATH}/scripts/linux/${CROSS_ARCH}-toolchain.cmake"
    )
fi
$VCS_PATH/scripts/utils/qt6_compile.sh "${COMPILE_ARGS[@]}"

echo "Patching Qt configuration"
cat << EOF > "$(pwd)/qt-linux/bin/qt.conf"
[Paths]
Prefix=..
EOF

cat << EOF > "$(pwd)/qt-linux/libexec/qt.conf"
[Paths]
Prefix=..
EOF

if [[ -z "$CROSS_ARCH" ]]; then
    echo "Bundling extra libs"
    for qttool in $(find "$(pwd)/bin" -executable -type f); do
        ldd $qttool | grep '=>' | awk '{print $3}' >> "$(pwd)/qt_build/qtlibdeps.txt"
    done
    for qtlibdep in $(sort -u "$(pwd)/qt_build/qtlibdeps.txt"); do
        cp -v $qtlibdep "$(pwd)/qt-linux/lib/"
        patchelf --set-rpath '$ORIGIN/../lib' "$(pwd)/qt-linux/lib/$(basename $qtlibdep)"
    done
fi

echo "Build Qt - Creating dist artifact"
echo $UPLOAD_DIR
tar -cJf "${UPLOAD_DIR}/${ARTIFACT_NAME}" qt-linux/
