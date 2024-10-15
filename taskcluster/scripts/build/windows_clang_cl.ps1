# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"
$QTPATH =resolve-path "$FETCHES_PATH/QT_OUT/"

# Prep Env:
# Switch to the work dir, configure qt
Set-Location -Path $TASK_WORKDIR
. "$FETCHES_PATH/QT_OUT/configure_qt.ps1"


# Ensure we are working from a full checkout of all submodules. Taskcluster
# doesn't handle recursive submodules, so we'll need to do it ourselves.
git -C "$REPO_ROOT_PATH" submodule update --init --recursive

# Setup Openssl Import
$SSL_PATH = "$FETCHES_PATH/QT_OUT/SSL"
if (Test-Path -Path $SSL_PATH) {
    $env:OPENSSL_ROOT_DIR = (resolve-path "$SSL_PATH").toString()
    $env:OPENSSL_USE_STATIC_LIBS = "TRUE"
}


## Use vendored rust crates, if present
if (Test-Path -Path "$FETCHES_PATH\cargo-vendor") {
    $CARGO_VENDOR_PATH = "$FETCHES_PATH/cargo-vendor" -replace @('\\', '/')
    New-Item -Path "$REPO_ROOT_PATH\.cargo" -ItemType "directory" -Force
@"
[source.vendored-sources]
directory = "$CARGO_VENDOR_PATH"

[source.crates-io]
replace-with = "vendored-sources"
"@ | Out-File -Encoding utf8 $REPO_ROOT_PATH\.cargo\config.toml
}

## Install MiniConda 
New-Item -ItemType Directory -Force -Path "$TASK_WORKDIR/miniconda"
$MINICONDA_DIR =resolve-path "$TASK_WORKDIR/miniconda"
Start-Process -NoNewWindow "$FETCHES_PATH\miniconda_installer.exe" -Wait -ArgumentList @('/S',"/D=$MINICONDA_DIR")
. $MINICONDA_DIR\shell\condabin\conda-hook.ps1

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
