# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"
$QTPATH =resolve-path "$FETCHES_PATH/QT_OUT/bin/"

New-Item -ItemType Directory -Force -Path "$TASK_WORKDIR/conda"
$CONDA_DIR =resolve-path "$TASK_WORKDIR/conda"
# Prep Env:
# Switch to the work dir, enable qt, enable msvc, enable rust
$env:PATH ="$FETCHES_PATH;$QTPATH;$env:PATH"
Set-Location -Path $TASK_WORKDIR
. "$FETCHES_PATH/QT_OUT/configure_qt.ps1"

# Setup Openssl Import
$SSL_PATH = resolve-path "$FETCHES_PATH/QT_OUT/SSL"
$env:OPENSSL_ROOT_DIR = (resolve-path "$SSL_PATH").toString()
$env:OPENSSL_USE_STATIC_LIBS = "TRUE"

# Extract the sources
$SOURCE_DSC = resolve-path "$FETCHES_PATH/mozillavpn_*.dsc"
$SOURCE_VERSION = ((select-string $SOURCE_DSC -Pattern '^Version:') -split " ")[1]
tar -xzvf (resolve-path "$FETCHES_PATH/mozillavpn_$SOURCE_VERSION.orig.tar.gz" -Relative)
$SOURCE_DIR = resolve-path "$TASK_WORKDIR/mozillavpn-$SOURCE_VERSION"


## Insall MiniConda 
Start-Process "$FETCHES_PATH/miniconda_installer.exe" -Wait -ArgumentList @('/S',"/D=$CONDA_DIR")
# We don't have the ability to do conda init - as that need's admin rights.
# So let's just do that ourselves :3
$Env:CONDA_EXE = "$CONDA_DIR\Scripts\conda.exe"
$Env:_CE_M = ""
$Env:_CE_CONDA = ""
$Env:_CONDA_ROOT = "$CONDA_DIR"
$Env:_CONDA_EXE = "$CONDA_DIR\Scripts\conda.exe"
$CondaModuleArgs = @{ChangePs1 = $False}
Import-Module "$Env:_CONDA_ROOT\shell\condabin\Conda.psm1" -ArgumentList $CondaModuleArgs
.\conda.exe "shell.powershell" "hook" | Out-String | ?{$_} | Invoke-Expression

# Conda is now ready - let's enable the env
conda env create --force -f $REPO_ROOT_PATH/env.yml
conda activate VPN
. "$REPO_ROOT_PATH/scripts/windows/conda_setup_win_sdk.ps1.ps1" # <- This download's all sdk things we need :3 
conda deactivate
conda activate VPN  # We should now be able to compile!

# We have not yet removed our VC_Redist strategy. Therefore we rely on the old vsstudio bundle to get us that :) 
$env:VCToolsRedistDir=(resolve-path "$FETCHES_PATH/VisualStudio/VC/Redist/MSVC/14.30.30704/").ToString()
# TODO: Remove this and change all to Microsoft_VC143 once we know there is no cavecat building with msvcv143
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x86.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC142_CRT_x86.msm

git submodule update --init --recursive


#Do not continune from this point on when we encounter an error
$ErrorActionPreference = "Stop"
mkdir $TASK_WORKDIR/cmake_build
$BUILD_DIR =resolve-path "$TASK_WORKDIR/cmake_build"

cmake --version
if ($env:MOZ_SCM_LEVEL -eq "3") {
    # Only on a release build we have access to those secrects.
    python3  $SOURCE_DIR/taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k sentry_dsn -f sentry_dsn
    python3  $SOURCE_DIR/taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k sentry_envelope_endpoint -f sentry_envelope_endpoint
    python3  $SOURCE_DIR/taskcluster/scripts/get-secret.py -s project/mozillavpn/tokens -k sentry_debug_file_upload_key -f sentry_debug_file_upload_key
    $SENTRY_ENVELOPE_ENDPOINT = Get-Content sentry_envelope_endpoint
    $SENTRY_DSN = Get-Content sentry_dsn
    #
    cmake -S $SOURCE_DIR -B $BUILD_DIR -GNinja -DCMAKE_BUILD_TYPE=Release -DSENTRY_DSN="$SENTRY_DSN" -DSENTRY_ENVELOPE_ENDPOINT="$SENTRY_ENVELOPE_ENDPOINT"
} else {
    # Do the generic build
   cmake -S $SOURCE_DIR -B $BUILD_DIR -GNinja -DCMAKE_BUILD_TYPE=Release
}
cmake --build $BUILD_DIR
cmake --build $BUILD_DIR --target msi
cmake --install $BUILD_DIR --prefix "$TASK_WORKDIR/unsigned"

Write-Output "Writing Artifacts"
New-Item -ItemType Directory -Path "$TASK_WORKDIR/artifacts" -Force
$ARTIFACTS_PATH =resolve-path "$TASK_WORKDIR/artifacts"
Copy-Item -Path $BUILD_DIR/windows/installer/MozillaVPN.msi -Destination $ARTIFACTS_PATH/MozillaVPN.msi
# Note: vc140.pdb is just the default name for pdb files (as we are using vc14x)
Copy-Item -Path "$BUILD_DIR/src/CMakeFiles/mozillavpn.dir/vc140.pdb" -Destination "$ARTIFACTS_PATH/Mozilla VPN.pdb"
Compress-Archive -Path $TASK_WORKDIR/unsigned/* -Destination $ARTIFACTS_PATH/unsigned.zip
Write-Output "Artifacts Location:$TASK_WORKDIR/artifacts"
Get-ChildItem -Path $TASK_WORKDIR/artifacts

if ($env:MOZ_SCM_LEVEL -eq "3") {
    sentry-cli-Windows-x86_64.exe login --auth-token $(Get-Content sentry_debug_file_upload_key)
    # This will ask sentry to scan all files in there and upload
    # missing debug info, for symbolification
    sentry-cli-Windows-x86_64.exe debug-files upload --org mozilla -p vpn-client $BUILD_DIR/src/CMakeFiles/mozillavpn.dir/vc140.pdb
}

# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "vctip.exe" -Force -ErrorAction SilentlyContinue

Write-Output "Open Processes:"

wmic process get description,executablepath
