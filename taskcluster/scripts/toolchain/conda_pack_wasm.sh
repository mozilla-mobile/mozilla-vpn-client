#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e
PATH=${PATH}:/opt/conda/bin

echo "Installing provided conda env..."
conda env create -f ${VCS_PATH}/env-wasm.yml
conda run -n vpn conda info
CONDA_VPN_PREFIX=$(conda env list | awk '{ if($1=="vpn") print $NF }')

# Copy the EMSDK fetch into the conda environment
EMSDK_SOURCE_DIR=$(find ${MOZ_FETCHES_DIR} -maxdepth 2 -name 'emscripten-releases-tags.json' -printf '%h\n')
EMSDK_VERSION=$(basename "${EMSDK_SOURCE_DIR}" | cut -d- -f2)
cp -r ${EMSDK_SOURCE_DIR} ${CONDA_VPN_PREFIX}/emsdk

# Install the EMSDK into the conda prefix
echo "Installing EMSDK..."
conda run -n vpn ${CONDA_VPN_PREFIX}/emsdk/emsdk install ${EMSDK_VERSION}
cat <<EOF > ${CONDA_VPN_PREFIX}/etc/conda/activate.d/emsdk-activate.sh
#!/bin/bash
export EMSDK_VERSION=${EMSDK_VERSION}
\${CONDA_PREFIX}/emsdk/emsdk activate \${EMSDK_VERSION}
source \${CONDA_PREFIX}/emsdk/emsdk_env.sh
EOF
chmod +x ${CONDA_VPN_PREFIX}/etc/conda/activate.d/emsdk-activate.sh

echo "Packing conda environment..."
mkdir -p ${UPLOAD_DIR}
conda-pack -n vpn -j $(nproc) --arcroot conda -o ${UPLOAD_DIR}/conda-wasm.tar.xz

echo "Done."
