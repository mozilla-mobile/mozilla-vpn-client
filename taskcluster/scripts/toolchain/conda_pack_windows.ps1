# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# This Script will build a conda-environment suitable to build 
# the vpn for windows. 
# It currently includes the windows-sdk, go, rust, c++ 
# compilers.
#
# Note: It does not ship QT right now. 

. "$PSScriptRoot/../common/helpers.ps1"

Install-MiniConda

conda env create --force -f $REPO_ROOT_PATH/env-windows.yml -n VPN
conda run -n VPN conda info
conda run -n VPN powershell -file "$REPO_ROOT_PATH\scripts\windows\conda-setup-xwin-sdk.ps1"

New-Item -ItemType Directory $TASK_WORKDIR\public\build
conda run -n VPN conda-pack --name VPN -o $TASK_WORKDIR\public\build\conda-windows.tar.gz
