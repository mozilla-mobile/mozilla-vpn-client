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

$TASKCLUSTER_LINK = "https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-win.latest/artifacts/public%2Fbuild%2Fqt6_win.zip"

$ProgressPreference = 'SilentlyContinue'
Set-Location $conda_folder 
Write-Output("Downloading qt artifact ...")
Invoke-WebRequest -Uri $TASKCLUSTER_LINK -OutFile "qt.zip" 
Write-Output("Expanding qt artifact ...")
Expand-Archive -Path "qt.zip" -DestinationPath "TaskCluster_QT"
$ProgressPreference = 'Continue'
Remove-Item "qt.zip" -ErrorAction SilentlyContinue

# Let conda find QT: 
conda env config vars set CMAKE_PREFIX_PATH="$conda_folder\\TaskCluster_QT\\QT_OUT\\lib\\cmake" | Out-Null
conda env config vars set OPENSSL_ROOT_DIR="$conda_folder\\TaskCluster_QT\\QT_OUT\\SSL" | Out-Null


Write-Output("You are SET! - Please re-activate your conda env to have stuff applied.")
Set-Location $OLD_PWD