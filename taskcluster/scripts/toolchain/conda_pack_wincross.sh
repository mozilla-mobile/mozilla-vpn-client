#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e
cd $TASK_WORKDIR

echo "Installing provided conda env..."
conda env create -f ${VCS_PATH}/env-windows.yml
conda run -n vpn conda info
CONDA_VPN_PREFIX=$(conda env list | awk '{ if($1=="vpn") print $NF }')

# The default conda installation of rust sets a CARGO_TARGET_<arch>_LINKER value
# that breaks cross compilation. So add an extra script to unset any cargo tampering.
mkdir -p ${CONDA_VPN_PREFIX}/etc/conda/activate.d/
cat <<EOF > ${CONDA_VPN_PREFIX}/etc/conda/activate.d/ZZ-unset-cargo-hacks.sh
#!/bin/sh
for NAME in \$(env | grep ^CARGO_TARGET_ | cut -d= -f1); do
    unset \${NAME}
done
EOF
chmod +x ${CONDA_VPN_PREFIX}/etc/conda/activate.d/ZZ-unset-cargo-hacks.sh

echo "Downloading x-win..."

# Fetch vsdownload.py from the msvc-wine repository.
XWIN_TAG="0.6.7"
XWIN_URL="https://github.com/Jake-Shadle/xwin/releases/download/${XWIN_TAG}/xwin-${XWIN_TAG}-x86_64-unknown-linux-musl.tar.gz"
curl -sSL $XWIN_URL -o xwin.tar.gz
tar xf xwin.tar.gz

echo "Downloading the windows SDK..."
./xwin-${XWIN_TAG}-x86_64-unknown-linux-musl/xwin \
    --arch aarch64 --arch x86_64 \
    --accept-license --manifest-version 17 splat \
    --include-debug-symbols --include-debug-libs \
    --use-winsysroot-style --preserve-ms-arch-notation \
    --output ${CONDA_VPN_PREFIX}/xwin

echo "Downloading Microsoft.VisualStudio.Component.VC.Redist.MSM"
python ${VCS_PATH}/scripts/windows/fetch-vsix-package.py \
    --manifest-version 17 --output ${CONDA_VPN_PREFIX}/xwin \
    Microsoft.VisualStudio.Component.VC.Redist.MSM

# Workaround for https://github.com/Jake-Shadle/xwin/issues/146
find ${CONDA_VPN_PREFIX} -type d -name 'lib' -printf '%h\n' | while read DIRNAME; do
    ln -s lib "${DIRNAME}/Lib"
done
find ${CONDA_VPN_PREFIX} -type d -name 'include' -printf '%h\n' | while read DIRNAME; do
    ln -s include "${DIRNAME}/Include"
done

echo "Packing conda environment..."
mkdir -p ${UPLOAD_DIR}
conda-pack -p ${CONDA_VPN_PREFIX} -j $(nproc) --arcroot conda -o ${UPLOAD_DIR}/conda-windows-cross.tar.xz

echo "Done."
