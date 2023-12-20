#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e

echo pwd 
ls 
cd vcs
ls 

chmod +x ${MOZ_FETCHES_DIR}/miniconda.sh
bash ${MOZ_FETCHES_DIR}/miniconda.sh -b -u -p .
source bin/activate

conda env create -f env.yml -n vpn
conda activate vpn
conda env config vars set QT_VERSION=${QT_VERSION}
./scripts/macos/conda_install_extras.sh
./scripts/macos/conda_setup_qt.sh

conda deactivate
conda activate vpn
echo "INFO"
conda info 
env
echo "INFO"
conda install conda-pack -y

mkdir -p ../../public/build
find ../../public/build/ -mindepth 1 -delete

conda-pack -p envs/vpn -o conda-ios.tar.gz
mv conda-ios.tar.gz  ../../public/build
