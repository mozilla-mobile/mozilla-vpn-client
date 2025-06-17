#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e
PATH=${PATH}:/opt/conda/bin

echo "Installing provided conda env..."
conda env create -f ${VCS_PATH}/env.yml
conda run -n vpn conda info
CONDA_VPN_PREFIX=$(conda env list | awk '{ if($1=="vpn") print $NF }')

echo "Installing MacOS SDK..."
conda run -n vpn ${VCS_PATH}/scripts/macos/macpkg.py ${MOZ_FETCHES_DIR}/cltools-macosnmos-sdk.pkg \
    --prefix Library/Developer/CommandLineTools/SDKs -o ${CONDA_VPN_PREFIX}
cat <<EOF > ${CONDA_VPN_PREFIX}/etc/conda/activate.d/01-macosx-sdkroot.sh
#!/bin/bash
export SDKROOT=\$(find \${CONDA_PREFIX} -name 'SDKSettings.plist' -printf '%h\n')
EOF
chmod +x ${CONDA_VPN_PREFIX}/etc/conda/activate.d/01-macosx-sdkroot.sh
cat ${CONDA_VPN_PREFIX}/etc/conda/activate.d/01-macosx-sdkroot.sh

echo "Installing MacOS toolchain..."
conda run -n vpn ${VCS_PATH}/scripts/macos/conda_install_osxcross.sh

echo "Installing Qt ${QT_VERSION} host tools..."
conda install -n vpn -y -c conda-forge qt6-main=${QT_VERSION}

echo "Installing conda-pack..."
conda install conda-pack -y

echo "Packing conda environment..."
mkdir -p ${UPLOAD_DIR}
conda-pack -n vpn -o ${UPLOAD_DIR}/conda-osxcross.tar.gz

echo "Done."
