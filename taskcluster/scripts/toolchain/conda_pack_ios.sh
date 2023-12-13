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

conda install conda-pack -y
conda env create -f env.yml -n vpn
bash -l -c "conda activate vpn && conda env config vars set QT_VERSION=${QT_VERSION}"
bash -l -c "conda activate vpn && ./scripts/macos/conda_install_extras.sh"
bash -l -c "conda activate vpn && ./scripts/macos/conda_setup_qt.sh"
    
mkdir -p ../../public/build
conda pack -n vpn -o $UPLOAD_DIR/conda-ios.tar.gz
