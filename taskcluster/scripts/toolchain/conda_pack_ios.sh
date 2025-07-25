#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e

echo pwd 
ls 
cd vcs
ls 



# save the passed QT_Version
# as that will be overwritten once 
# we enable the env-apple.yml
BACKUP_QT_VERSION=${QT_VERSION}
chmod +x ${MOZ_FETCHES_DIR}/miniconda.sh
bash ${MOZ_FETCHES_DIR}/miniconda.sh -b -u -p .
source bin/activate

# Normally pip is locked down to only 
# allow download from moz-pip mirror. 
# The packages are firefox only and everything
# we're going to fetch is pinned down, so 
# let's remove that restriction for the current task. 
rm -f ~/.config/pip/pip.conf
pip config --user set install.no-index 0 
pip config debug

conda env create -f env-apple.yml -n vpn
conda activate vpn
echo "SETTING QT_VERSION=${BACKUP_QT_VERSION}"
conda env config vars set QT_VERSION=${BACKUP_QT_VERSION}
# Re-enable to apply
env
conda deactivate

conda run -n vpn ./scripts/macos/conda_setup_qt.sh
conda run -n vpn conda info
conda install conda-pack -y

mkdir -p ../../public/build
find ../../public/build/ -mindepth 1 -delete

conda-pack -p envs/vpn -j $(sysctl -n hw.logicalcpu) -o conda-ios.tar.xz
mv conda-ios.tar.xz  ../../public/build

# remove our Pip conf, so the restrictions are back. 
rm -f ~/.config/pip/pip.conf
