# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"
$QTPATH =resolve-path "$FETCHES_PATH/QT_OUT/"

# Install Miniconda
New-Item -ItemType Directory -Force -Path "$TASK_WORKDIR/conda"
$CONDA_DIR =resolve-path "$TASK_WORKDIR/conda"
Start-Process "$FETCHES_PATH/miniconda_installer.exe" -Wait -ArgumentList @('/S',"/D=$CONDA_DIR")
. "$CONDA_DIR/shell/condabin/conda-hook.ps1"

# Prep Env:
# Switch to the work dir, configure qt
Set-Location -Path $TASK_WORKDIR
. "$FETCHES_PATH/QT_OUT/configure_qt.ps1"


# We have not yet removed our VC_Redist strategy. Therefore we rely on the old vsstudio bundle to get us that :)
# TODO: We need to handle this at some point.
$env:VCToolsRedistDir=(resolve-path "$FETCHES_PATH/VisualStudio/VC/Redist/MSVC/14.30.30704/").ToString()
# TODO: Remove this and change all to Microsoft_VC143 once we know there is no cavecat building with msvcv143
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x86.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC142_CRT_x86.msm


# Setup Openssl Import
$SSL_PATH = "$FETCHES_PATH/QT_OUT/SSL"
if (Test-Path -Path $SSL_PATH) {
    $env:OPENSSL_ROOT_DIR = (resolve-path "$SSL_PATH").toString()
    $env:OPENSSL_USE_STATIC_LIBS = "TRUE"
}


# Extract the sources
$SOURCE_DSC = resolve-path "$FETCHES_PATH/mozillavpn_*.dsc"
$SOURCE_VERSION = ((select-string $SOURCE_DSC -Pattern '^Version:') -split " ")[1]
tar -xzvf (resolve-path "$FETCHES_PATH/mozillavpn_$SOURCE_VERSION.orig.tar.gz" -Relative)
$SOURCE_DIR = resolve-path "$TASK_WORKDIR/mozillavpn-$SOURCE_VERSION"


## Setup the conda environment
$FETCHES_PATH/Scripts/conda-unpack
conda activate $FETCHES_PATH
conda info


# Okay We are ready to build!
mkdir $TASK_WORKDIR/cmake_build
$BUILD_DIR =resolve-path "$TASK_WORKDIR/cmake_build"

# Do the generic build
cmake -S $SOURCE_DIR -B $BUILD_DIR -GNinja `
        -DCMAKE_BUILD_TYPE=Release `
        -DPYTHON_EXECUTABLE="$CONDA_PREFIX\python.exe" `
        -DGOLANG_BUILD_TOOL="$CONDA_PREFIX\bin\go.exe" `
        -DWIREGUARD_FOLDER="$FETCHES_PATH\wireguard-nt" `
        -DCMAKE_PREFIX_PATH="$QTPATH/lib/cmake" `
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
python  $SOURCE_DIR/taskcluster/scripts/get-secret.py -s project/mozillavpn/level-1/sentry -k sentry_debug_file_upload_key -f sentry_debug_file_upload_key
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
