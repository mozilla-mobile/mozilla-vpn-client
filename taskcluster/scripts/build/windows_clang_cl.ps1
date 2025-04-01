# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. "$PSScriptRoot/../common/helpers.ps1"


$QTPATH =resolve-path "$FETCHES_PATH/QT_OUT/"

# Prep Env:
# Switch to the work dir, configure qt
Set-Location -Path $TASK_WORKDIR
. "$FETCHES_PATH/QT_OUT/configure_qt.ps1"


# Ensure we are working from a full checkout of all submodules. Taskcluster
# doesn't handle recursive submodules, so we'll need to do it ourselves.
git -C "$REPO_ROOT_PATH" submodule update --init --recursive


## Use vendored rust crates, if present
Add-CargoVendor
## Install MiniConda 
Install-MiniConda 
## Unpack and activate the conda environment.
$CONDA_DIR = resolve-path "$FETCHES_PATH\conda-windows"
Start-Process -NoNewWindow "$CONDA_DIR\Scripts\conda-unpack.exe" -Wait
conda activate $CONDA_DIR
conda info

# Okay We are ready to build!
mkdir $TASK_WORKDIR/cmake_build
$BUILD_DIR =resolve-path "$TASK_WORKDIR/cmake_build"

# Do the generic build
cmake -S $REPO_ROOT_PATH -B $BUILD_DIR -GNinja `
        -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_TOOLCHAIN_FILE="scripts/windows/conda-toolchain.cmake" `
        -DWIREGUARD_FOLDER="$FETCHES_PATH\wireguard-nt" `
        -DCMAKE_PREFIX_PATH="$QTPATH/lib/cmake" `
        -DCMAKE_TOOLCHAIN_FILE=scripts/windows/conda-toolchain.cmake `
        -DBUILD_TESTS=OFF

cmake --build $BUILD_DIR

## Building the MSI Installer.
cmake --build $BUILD_DIR --target msi

cmake --install $BUILD_DIR --prefix "$TASK_WORKDIR/unsigned"

Write-Output "Writing Artifacts"
New-Item -ItemType Directory -Path "$TASK_WORKDIR/artifacts" -Force
$ARTIFACTS_PATH =resolve-path "$TASK_WORKDIR/artifacts"
Copy-Item -Path $BUILD_DIR/windows/installer/MozillaVPN.msi -Destination $ARTIFACTS_PATH/MozillaVPN.msi

Compress-Archive -Path $TASK_WORKDIR/unsigned/* -Destination $ARTIFACTS_PATH/unsigned.zip
Write-Output "Artifacts Location:$TASK_WORKDIR/artifacts"
Get-ChildItem -Path $TASK_WORKDIR/artifacts


Get-command python
python  $REPO_ROOT_PATH/taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/sentry -k sentry_debug_file_upload_key -f sentry_debug_file_upload_key
$env:SENTRY_AUTH_TOKEN=$(Get-Content sentry_debug_file_upload_key)
# Are we logged in?
sentry-cli-Windows-x86_64.exe info

# This will ask sentry to scan all files in there and upload
# missing debug info, for symbolification

sentry-cli-Windows-x86_64.exe debug-files check "$BUILD_DIR\src\Mozilla VPN.pdb"
Get-ChildItem $TASK_WORKDIR/unsigned/
if ($env:MOZ_SCM_LEVEL -eq "3") {
    sentry-cli-Windows-x86_64.exe debug-files upload --org mozilla -p vpn-client --include-sources $BUILD_DIR
} else{
    sentry-cli-Windows-x86_64.exe --log-level info debug-files upload --org mozilla -p vpn-client --include-sources --no-upload $BUILD_DIR
}
