#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

/opt/conda/bin/conda install conda-pack

/opt/conda/bin/conda env create -f /root/env.yml -n vpn
bash -l -c "conda activate vpn && ./scripts/android/conda_setup_sdk.sh"
bash -l -c "conda activate vpn && ./scripts/android/conda_setup_qt.sh"
bash -l -c "conda activate vpn && ./scripts/android/conda_trim.sh"
    
mkdir -p ../../public/build
/opt/conda/bin/conda pack -n vpn -o ../../public/build/conda_android.tar.gz

