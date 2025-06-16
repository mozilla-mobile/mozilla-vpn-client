#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e
PATH=${PATH}:/opt/conda/bin

echo "Installing provided conda env..."
conda env create -f ${VCS_PATH}/env.yml
conda run -n vpn ${VCS_PATH}/scripts/macos/conda_install_osxcross.sh
conda run -n vpn conda info

echo "Installing conda-pack..."
conda install conda-pack -y

echo "Packing conda environment..."
mkdir -p ${UPLOAD_DIR}/public/build
conda-pack -n vpn -o ${UPLOAD_DIR}/public/build/conda-osxcross.tar.gz

echo "Done."
