# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"


function Install-MiniConda {
    if (!(Test-Path -Path "$FETCHES_PATH\miniconda_installer.exe") ) {
        Write-Error "Miniconda installer was not fetched: $FETCHES_PATH\miniconda_installer.exe"
        return 
    }
    ## Install MiniConda 
    New-Item -ItemType Directory -Force -Path "$TASK_WORKDIR/miniconda"
    $MINICONDA_DIR =resolve-path "$TASK_WORKDIR/miniconda"
    Start-Process -NoNewWindow "$FETCHES_PATH\miniconda_installer.exe" -Wait -ArgumentList @('/S',"/D=$MINICONDA_DIR")
    . $MINICONDA_DIR\shell\condabin\conda-hook.ps1

}

function Add-CargoVendor {
    if (!(Test-Path -Path "$FETCHES_PATH\cargo-vendor") ) {
        Write-Output "No Cargo Vendor Fetched!"
        return 
    }
        $CARGO_VENDOR_PATH = "$FETCHES_PATH/cargo-vendor" -replace @('\\', '/')
        New-Item -Path "$REPO_ROOT_PATH\.cargo" -ItemType "directory" -Force
@"
[source.vendored-sources]
directory = "$CARGO_VENDOR_PATH"
    
[source.crates-io]
replace-with = "vendored-sources"
"@ | Out-File -Encoding utf8 $REPO_ROOT_PATH\.cargo\config.toml
    Write-Output "Added $CARGO_VENDOR_PATH as Cargo Vendor Path"
}
