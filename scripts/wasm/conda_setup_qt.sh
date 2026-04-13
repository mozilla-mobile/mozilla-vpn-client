#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e

BASE_PREFIX=$(conda info --base)
if [[ "$CONDA_PREFIX" == "$BASE_PREFIX" ]]; then
    echo "Please run this script inside a non-base conda environment."
    exit 1
fi

if [[ "$(uname)" != "Linux" ]]; then
    echo "WASM builds are only supported on Linux."
    exit 1
fi

if [[ -z "$QT_VERSION" ]]; then
    echo "Error: QT_VERSION environment variable is not set."
    echo "Hint: it is set automatically when using env-wasm.yml."
    exit 1
fi

export QT_DIR="${CONDA_PREFIX}/Qt"

echo "Installing Qt ${QT_VERSION} host tools..."
python -m aqt install-qt --outputdir "${QT_DIR}" linux desktop "${QT_VERSION}" linux_gcc_64 -m qtshadertools qtlottie
echo "Installing Qt ${QT_VERSION} for WASM..."
python -m aqt install-qt --outputdir "${QT_DIR}" all_os wasm "${QT_VERSION}" wasm_singlethread -m all

chmod +x "${QT_DIR}/${QT_VERSION}/wasm_singlethread/bin/qt-cmake"

# Activation hooks
mkdir -p "${CONDA_PREFIX}/etc/conda/activate.d/"
mkdir -p "${CONDA_PREFIX}/etc/conda/deactivate.d/"

cat > "${CONDA_PREFIX}/etc/conda/activate.d/vpn_wasm_qt.sh" <<EOF
#!/bin/bash
export QT_HOST_PATH=\${CONDA_PREFIX}/Qt/${QT_VERSION}/gcc_64
export QT_HOST_PATH_CMAKE_DIR=\${CONDA_PREFIX}/Qt/${QT_VERSION}/gcc_64/lib/cmake
export QTPATH=\${CONDA_PREFIX}/Qt/${QT_VERSION}/wasm_singlethread
EOF
chmod +x "${CONDA_PREFIX}/etc/conda/activate.d/vpn_wasm_qt.sh"

cat > "${CONDA_PREFIX}/etc/conda/deactivate.d/vpn_wasm_qt.sh" <<EOF
#!/bin/bash
unset QT_HOST_PATH
unset QT_HOST_PATH_CMAKE_DIR
unset QTPATH
EOF
chmod +x "${CONDA_PREFIX}/etc/conda/deactivate.d/vpn_wasm_qt.sh"

# qt-cmake wrapper that passes WASM cross-compilation flags automatically
cat > "${CONDA_PREFIX}/bin/qt-cmake" <<EOF
#!/bin/bash
\${CONDA_PREFIX}/Qt/${QT_VERSION}/wasm_singlethread/bin/qt-cmake \\
    -DQT_HOST_PATH=\${QT_HOST_PATH} \\
    -DQT_HOST_PATH_CMAKE_DIR=\${QT_HOST_PATH_CMAKE_DIR} \\
    "\$@"
EOF
chmod +x "${CONDA_PREFIX}/bin/qt-cmake"

echo "Qt ${QT_VERSION} WASM setup complete."
echo "Please re-activate the conda env to apply changes:"
echo "  conda deactivate && conda activate vpn"
