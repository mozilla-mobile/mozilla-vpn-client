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

EMSDK_VERSION="${EMSDK_VERSION:-4.0.7}"

echo "Installing EMSDK ${EMSDK_VERSION} into ${CONDA_PREFIX}..."

# Download and extract EMSDK
curl -L "https://github.com/emscripten-core/emsdk/archive/refs/tags/${EMSDK_VERSION}.tar.gz" \
    | tar -xz -C "${CONDA_PREFIX}"
mv "${CONDA_PREFIX}/emsdk-${EMSDK_VERSION}" "${CONDA_PREFIX}/emsdk"

# Install and activate the SDK
"${CONDA_PREFIX}/emsdk/emsdk" install "${EMSDK_VERSION}"
"${CONDA_PREFIX}/emsdk/emsdk" activate "${EMSDK_VERSION}"

# Create activation hook so EMSDK env vars are set on `conda activate`
mkdir -p "${CONDA_PREFIX}/etc/conda/activate.d/"
mkdir -p "${CONDA_PREFIX}/etc/conda/deactivate.d/"

cat > "${CONDA_PREFIX}/etc/conda/activate.d/emsdk-activate.sh" <<EOF
#!/bin/bash
export EMSDK_VERSION=${EMSDK_VERSION}
\${CONDA_PREFIX}/emsdk/emsdk activate \${EMSDK_VERSION}
source \${CONDA_PREFIX}/emsdk/emsdk_env.sh
EOF
chmod +x "${CONDA_PREFIX}/etc/conda/activate.d/emsdk-activate.sh"

cat > "${CONDA_PREFIX}/etc/conda/deactivate.d/emsdk-deactivate.sh" <<EOF
#!/bin/bash
unset EMSDK
unset EMSDK_NODE
unset EMSDK_PYTHON
unset EMSDK_VERSION
EOF
chmod +x "${CONDA_PREFIX}/etc/conda/deactivate.d/emsdk-deactivate.sh"

echo "EMSDK ${EMSDK_VERSION} installed. Please re-activate the conda env to apply changes:"
echo "  conda deactivate && conda activate vpn"
