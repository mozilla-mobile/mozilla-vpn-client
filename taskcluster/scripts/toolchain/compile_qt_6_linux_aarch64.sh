#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

#
# Inputs (via MOZ_FETCHES_DIR):
#   qt-everywhere-src-*   - Qt6 source tarball
#   qt-host-tools/        - Qt6 host tools for the same version
#
# Output (via UPLOAD_DIR):
#   qt6_linux_aarch64.tar.xz

set -e
cd $HOME
export XZ_DEFAULTS="-T0"

QT_SOURCE_DIR_ORIG=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
QT_SOURCE_VERSION=$(echo $QT_SOURCE_DIR_ORIG | awk -F"-" '{print $NF}')
QT_HOST_TOOLS="${MOZ_FETCHES_DIR}/qt-host-tools"
BUILD_DIR="$(pwd)/qt_build_aarch64"
INSTALL_DIR="$(pwd)/qt-linux-aarch64"

if [[ ! -d "${QT_HOST_TOOLS}" ]]; then
    echo "ERROR: qt-host-tools not found at ${QT_HOST_TOOLS}" >&2
    echo "This toolchain requires the qt-tools-6.10 toolchain artifact." >&2
    exit 1
fi

# Copy Qt source to a local writable directory.
# patch(1) needs to create temp files next to the files it modifies; if the
# source came from a read-only mount (e.g. local testing) that fails.
# cp -a is also needed because Qt configure writes into the source tree.
QT_SOURCE_DIR="$(pwd)/qt-src"
echo "Copying Qt source to writable working directory..."
cp -a "${QT_SOURCE_DIR_ORIG}" "${QT_SOURCE_DIR}"

# Apply known patches (same as the x86_64 Qt build)
if [[ $(echo -e "${QT_SOURCE_VERSION}\n6.10.3" | sort --version-sort | head -1) != "6.10.3" ]]; then
    echo "Patching for QTBUG-141830"
    patch -d "${QT_SOURCE_DIR}/qtdeclarative" -p1 < \
        "${VCS_PATH}/taskcluster/scripts/toolchain/patches/qtbug-141830-qsortfilterproxymodel.patch"
fi

# Write the aarch64 CMake toolchain file to a known location for configure.
TOOLCHAIN_FILE="${VCS_PATH}/scripts/linux/aarch64-toolchain.cmake"

echo "Configuring Qt6 for aarch64 cross-compilation..."
mkdir -p "${BUILD_DIR}" "${INSTALL_DIR}"
INSTALL_DIR=$(cd "${INSTALL_DIR}" && pwd)

# The Qt configure script separates native configure flags (before --) from
# CMake flags (after --). The toolchain file is passed via CMake.
# -DCMAKE_STAGING_PREFIX tells CMake where to install on the host filesystem,
# while --prefix is embedded in Qt's qmake/cmake config as the target install path.
(cd "${BUILD_DIR}" && bash "${QT_SOURCE_DIR}/configure" \
    --prefix="${INSTALL_DIR}" \
    -qt-host-path "${QT_HOST_TOOLS}" \
    -opensource \
    -confirm-license \
    -release \
    -static \
    -strip \
    -silent \
    -nomake tests \
    -make libs \
    -no-feature-sql-odbc \
    -no-feature-qtattributionsscanner \
    -no-feature-qtdiag \
    -no-feature-qtplugininfo \
    -no-feature-pixeltool \
    -no-feature-distancefieldgenerator \
    -no-feature-assistant \
    -no-feature-tiff \
    -no-feature-webp \
    -no-feature-cups \
    -no-feature-textmarkdownwriter \
    -no-feature-itemmodeltester \
    -no-feature-sql-sqlite \
    -no-feature-sql \
    -skip qt3d \
    -skip qtdoc \
    -skip qtgrpc \
    -skip qtconnectivity \
    -skip qtquickeffectmaker \
    -skip qtquicktimeline \
    -skip qtwebengine \
    -skip qtwebview \
    -skip qtlocation \
    -skip qtmultimedia \
    -skip qtserialport \
    -skip qtsensors \
    -skip qtgamepad \
    -skip qtgraphs \
    -skip qtandroidextras \
    -skip qtquick3dphysics \
    -skip qtactiveqt \
    -skip qtcharts \
    -skip qtcoap \
    -skip qtdatavis3d \
    -skip qtremoteobjects \
    -skip qtlottie \
    -skip qtmqtt \
    -skip qtopcua \
    -skip qtpositioning \
    -skip qtquick3d \
    -skip qtscxml \
    -skip qtserialbus \
    -skip qtspeech \
    -skip qtvirtualkeyboard \
    -feature-imageformat_png \
    -feature-optimize_full \
    -feature-xml \
    -qt-doubleconversion \
    -qt-libpng \
    -qt-zlib \
    -qt-pcre \
    -openssl-runtime \
    -egl \
    -opengl es2 \
    -no-icu \
    -no-linuxfb \
    -system-freetype \
    -fontconfig \
    -bundled-xcb-xinput \
    -feature-qdbus \
    -feature-wayland \
    -no-feature-gssapi \
    -no-feature-zstd \
    -xcb \
    -- \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_STAGING_PREFIX="${INSTALL_DIR}" \
    -DCMAKE_FIND_ROOT_PATH="/" \
    -DCMAKE_LIBRARY_ARCHITECTURE=aarch64-linux-gnu \
) || { echo "Qt configure failed" >&2; exit 1; }

echo "Building Qt6 for aarch64..."
cmake --build "${BUILD_DIR}" --parallel $(nproc) \
    || { echo "Qt build failed" >&2; exit 1; }

echo "Installing Qt6 for aarch64..."
cmake --install "${BUILD_DIR}" \
    || { echo "Qt install failed" >&2; exit 1; }

echo "Patching Qt configuration for relocation..."
cat << EOF > "${INSTALL_DIR}/bin/qt.conf"
[Paths]
Prefix=..
EOF

cat << EOF > "${INSTALL_DIR}/libexec/qt.conf"
[Paths]
Prefix=..
EOF

echo "Creating distribution artifact..."
mkdir -p "${UPLOAD_DIR}"
tar -cJf "${UPLOAD_DIR}/qt6_linux_aarch64.tar.xz" qt-linux-aarch64/

echo "Qt6 aarch64 cross-compilation complete."
