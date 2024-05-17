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
conda create -y -n qt
conda activate qt
conda install -y -c conda-forge cmake=3.26.3 ninja=1.11.0

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
echo "Building $(basename $QT_SOURCE_DIR)"
$VCS_PATH/scripts/utils/qt6_compile.sh $QT_SOURCE_DIR ${TASK_WORKDIR}/qt_dist

echo "Creating Qt dist artifact"
mkdir -p ${TASK_WORKDIR}/public/build
zip -qr ${TASK_WORKDIR}/public/build/qt6_mac.zip ${TASK_WORKDIR}/qt_dist/*
