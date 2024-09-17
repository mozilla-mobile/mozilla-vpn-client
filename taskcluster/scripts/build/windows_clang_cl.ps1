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


# We have not yet removed our VC_Redist strategy. Therefore we rely on the old vsstudio bundle to get us that :)
# TODO: We need to handle this at some point.
$env:VCToolsRedistDir=(resolve-path "$FETCHES_PATH/VisualStudio/VC/Redist/MSVC/14.30.30704/").ToString()
# TODO: Remove this and change all to Microsoft_VC143 once we know there is no cavecat building with msvcv143
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x86.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC142_CRT_x86.msm


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


## Setup the conda environment
. $REPO_ROOT_PATH/scripts/utils/call_bat.ps1  $FETCHES_PATH/Scripts/activate.bat
conda-unpack

# Conda Pack excpets to be run under cmd. therefore it will
# (unlike conda) ignore activate.d powershell scripts.
# So let's manually run the activation scripts.
#
$CONDA_PREFIX = $env:CONDA_PREFIX

$ACTIVATION_SCRIPTS = Get-ChildItem -Path "$CONDA_PREFIX\etc\conda\activate.d" -Filter "*.ps1"
foreach ($script in  $ACTIVATION_SCRIPTS)  {
    Write-Output "Activating: $CONDA_PREFIX\etc\conda\activate.d\$script"
    . "$CONDA_PREFIX\etc\conda\activate.d\$script"
}
# This is a wierd bug `PREFIX/bin` does not seem to be on the PATH
# when we run the activate.bat :shrugs:
# This will cause go to be missing.
$env:PATH="$CONDA_PREFIX\bin;$env:Path"
gci env:* | sort-object name


# Okay We are ready to build!
mkdir $TASK_WORKDIR/cmake_build
$BUILD_DIR =resolve-path "$TASK_WORKDIR/cmake_build"

# Do the generic build
cmake -S $REPO_ROOT_PATH -B $BUILD_DIR -GNinja `
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
