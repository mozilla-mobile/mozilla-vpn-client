#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e

echo pwd 
ls 
cd vcs
ls 

/opt/conda/bin/conda install conda-pack -y

/opt/conda/bin/conda env create -f env.yml -n vpn
bash -l -c "conda activate vpn && conda env config vars set QT_VERSION=${QT_VERSION}"
bash -l -c "conda activate vpn && ./scripts/android/conda_setup_sdk.sh"
bash -l -c "conda activate vpn && ./scripts/android/conda_setup_qt.sh"
bash -l -c "conda activate vpn && ./scripts/android/conda_trim.sh"
    
mkdir -p ../../public/build
/opt/conda/bin/conda pack -n vpn -o $UPLOAD_DIR/conda-android.tar.gz

