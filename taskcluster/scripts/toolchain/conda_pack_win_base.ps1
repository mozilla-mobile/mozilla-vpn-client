# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This Script will build a conda-environment containing non-compiler essentials
# needed to build (cmake, python)
# It does not ship a compiler not a Windows SDK
#

. "$PSScriptRoot/../common/helpers.ps1"

Install-MiniConda

## Conda is now ready - let's enable the env
conda env create --force -f  $REPO_ROOT_PATH/taskcluster/conda/env-win-base.yml

New-Item -ItemType Directory $TASK_WORKDIR\public\build
conda run -n win-base conda-pack --name win-base -o $TASK_WORKDIR\public\build\conda-windows.tar.gz
