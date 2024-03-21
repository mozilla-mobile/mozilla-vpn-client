#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e
cd ${TASK_WORKDIR}

if [ -e ${TASK_WORKDIR}/miniconda ]; then
    echo "Conda already exist? Remove it."
    rm -rf ${TASK_WORKDIR}/miniconda
fi

echo "Installing conda"
chmod +x ${MOZ_FETCHES_DIR}/miniconda.sh
bash ${MOZ_FETCHES_DIR}/miniconda.sh -b -p ${TASK_WORKDIR}/miniconda
source ${TASK_WORKDIR}/miniconda/bin/activate

echo "Installing provided conda env..."
conda env create -f ${VCS_PATH}/env.yml
conda activate VPN
${VCS_PATH}/scripts/macos/conda_install_extras.sh  
conda info
conda deactivate

echo "Installing conda-pack..."
conda install conda-pack -y

echo "Packing conda environment..."
if [ -e ${TASK_WORKDIR}/public/build ]; then
    rm -rf ${TASK_WORKDIR}/public/build
fi
mkdir -p ${TASK_WORKDIR}/public/build
conda-pack -p ${TASK_WORKDIR}/miniconda/envs/vpn -o ${TASK_WORKDIR}/public/build/conda-macos.tar.gz

echo "Done."
