# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script will fetch the latest 
# QT build used in CI and setup conda
# so that you can use it to build the client.
$conda_env = conda info --json | ConvertFrom-Json

if( $conda_env.active_prefix_name -eq "base"){
    Write-Output("Not in an active conda env. abort")
    return -1
}
$conda_folder = $conda_env.active_prefix 
Write-Output("Installing in $conda_folder")
$OLD_PWD = $PWD # Backup that to go back once we done. 
Set-Location $conda_folder 

# Get QT version from conda environment variable
$QT_VERSION = $env:QT_VERSION
if (-not $QT_VERSION) {
     Write-Output("QT_VERSION not found in conda environment. abort")
     Set-Location $OLD_PWD
     return -1
}
Write-Output("Using QT version: $QT_VERSION")

# Parse version to get major.minor format (e.g., "6.10.1" -> "6.10")
$version = [System.Version]$QT_VERSION
$QT_VERSION_SHORT = "$($version.Major).$($version.Minor)"

# Construct the index and download URLs
$TASKCLUSTER_LINK ="https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-windows-x86_64-$QT_VERSION_SHORT.latest/artifacts/public%2Fbuild%2Fqt6_win.tar.xz"

Set-Location $conda_folder 
Write-Output("Downloading qt artifact ...")
curl.exe -L -o "qt6_win.tar.xz" $TASKCLUSTER_LINK
Write-Output("Extracting qt artifact ...")
tar -xf "qt6_win.tar.xz"
Remove-Item "qt6_win.tar.xz" -ErrorAction SilentlyContinue

# Let conda find QT: 

# For that we add an activation script. 
#

$activate = @"
`$env:CMAKE_PREFIX_PATH="`$env:CONDA_PREFIX\qt-windows\lib\cmake"
`$env:QT_HOST_PATH="`$env:CONDA_PREFIX\qt-windows\"
"@
Out-File -Encoding utf8 `
         -FilePath  "$conda_folder\etc\conda\activate.d\vpn_qt.ps1"`
         -InputObject $activate 


$deactivate = @"
Remove-Item Env:\QT_HOST_PATH
Remove-Item Env:\CMAKE_PREFIX_PATH
"@
Out-File -Encoding utf8 `
         -FilePath  "$conda_folder\etc\conda\deactivate.d\vpn_qt.ps1"`
         -InputObject $deactivate 


Write-Output("You are SET! - Please re-activate your conda env to have stuff applied.")
Set-Location $OLD_PWD
