# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script will fetch ${env:QT_VERSION} QT from a mirror. 
$conda_env = conda info --json | ConvertFrom-Json

if( $conda_env.active_prefix_name -eq "base"){
    Write-Output("Not in an active conda env. abort")
    return -1
}
$conda_folder = $conda_env.active_prefix 
Write-Output("Installing in $conda_folder")
$OLD_PWD = $PWD # Backup that to go back once we done. 
Set-Location $conda_folder 

$arch ="win64_msvc2019_64"
$HOST_FOLDER_NAME="msvc2019_64"


# QT_Host Tools
python -m aqt install-qt windows desktop ${env:QT_VERSION} $arch -m all --outputdir Qt


# For that we add an activation script. 
#

$activate = @"
`$env:CMAKE_PREFIX_PATH="`$env:CONDA_PREFIX\Qt\`$env:QT_VERSION\$HOST_FOLDER_NAME\lib\cmake"
`$env:QT_HOST_PATH="`$env:CONDA_PREFIX\Qt\`$env:QT_VERSION\$HOST_FOLDER_NAME"
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
