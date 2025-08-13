#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e

echo "Installing provided conda env..."
conda env create -f ${VCS_PATH}/env-apple.yml
conda run -n vpn conda info
CONDA_VPN_PREFIX=$(conda env list | awk '{ if($1=="vpn") print $NF }')

# The default conda installation of rust sets a CARGO_TARGET_<arch>_LINKER value
# that breaks cross compilation. So add an extra script to unset any cargo tampering.
cat <<EOF > ${CONDA_VPN_PREFIX}/etc/conda/activate.d/ZZ-unset-cargo-hacks.sh
#!/bin/sh
for NAME in \$(env | grep ^CARGO_TARGET_ | cut -d= -f1); do
    unset \${NAME}
done
EOF
chmod +x ${CONDA_VPN_PREFIX}/etc/conda/activate.d/ZZ-unset-cargo-hacks.sh

echo "Installing MacOS SDK..."
conda run -n vpn ${VCS_PATH}/scripts/macos/macpkg.py ${MOZ_FETCHES_DIR}/cltools-macosnmos-sdk.pkg \
    --prefix Library/Developer/CommandLineTools/SDKs -o ${CONDA_VPN_PREFIX}
cat <<EOF > ${CONDA_VPN_PREFIX}/etc/conda/activate.d/01-macosx-sdkroot.sh
#!/bin/bash
export SDKROOT=\$(find \${CONDA_PREFIX} -name 'SDKSettings.plist' -printf '%h\n')
EOF
chmod +x ${CONDA_VPN_PREFIX}/etc/conda/activate.d/01-macosx-sdkroot.sh
cat <<EOF > ${CONDA_VPN_PREFIX}/etc/conda/deactivate.d/01-macosx-sdkroot.sh
#!/bin/bash
unset SDKROOT
EOF
chmod +x ${CONDA_VPN_PREFIX}/etc/conda/deactivate.d/01-macosx-sdkroot.sh

echo "Packing conda environment..."
mkdir -p ${UPLOAD_DIR}
conda-pack -n vpn -j $(nproc) --arcroot conda -o ${UPLOAD_DIR}/conda-osxcross.tar.xz

echo "Done."
