#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e
set -x

. $(dirname $0)/../../../scripts/utils/commons.sh

# Find the Output Directory and clear that
TASK_HOME=$(dirname "${MOZ_FETCHES_DIR}" )
rm -rf "${TASK_HOME}/miniconda"
mkdir -p "${TASK_HOME}/miniconda"

print Y "Installing conda"
chmod +x ${MOZ_FETCHES_DIR}/miniconda.sh
bash ${MOZ_FETCHES_DIR}/miniconda.sh -b -u -p ${TASK_HOME}/miniconda
source ${TASK_HOME}/miniconda/bin/activate


print Y "Installing provided conda env..."
# TODO: Check why --force is needed if we install into TASK_HOME?
conda env create --force -f env.yml    
conda install conda-pack
conda pack -n VPN -o ${TASK_WORKDIR}/public/build/conda_env_mac.tar.gz
