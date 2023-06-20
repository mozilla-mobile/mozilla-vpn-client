# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


$X_WIN_VERSION = "0.2.10"

$conda_env = conda info --json | ConvertFrom-Json
$OLD_PWD = $PWD # Backup that to go back once we done. 

if( $conda_env.active_prefix_name -eq "base"){
    Write-Output("Not in an active conda env. abort")
    return -1
}
$conda_folder = $conda_env.active_prefix 
