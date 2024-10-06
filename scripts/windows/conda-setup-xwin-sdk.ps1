# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


$X_WIN_VERSION = "0.6.5"

$conda_env = conda info --json | ConvertFrom-Json

if( $conda_env.active_prefix_name -eq "base"){
    Write-Output("Not in an active conda env. abort")
    return -1
}
$conda_folder = $conda_env.active_prefix 
Write-Output("Downloading x-win")
# Small X-Win appretiation comment. 
# It's really great, 
# it can process windows store manifests, request files from MS-servers and unpack it. 
# we can use it to get the offical SDK files from microsoft. 
# We can't cargo-install it because to use the clang-msvc compat mode, 
# 
Invoke-WebRequest -Uri "https://github.com/Jake-Shadle/xwin/releases/download/$X_WIN_VERSION/xwin-$X_WIN_VERSION-x86_64-pc-windows-msvc.tar.gz" -OutFile "$conda_folder\xwin.tar.gz"  
$ProgressPreference = 'Continue'
Write-Output("Unpack x-win")
Start-Process -WorkingDirectory "$conda_folder" -Wait tar -ArgumentList @('-xf', "xwin.tar.gz")

# Splat the CRT and SDK file to /xwin/crt and /xwin/sdk respectively
Write-Output("Downloading the windows SDK")
$env:PATH ="$conda_folder\xwin-$X_WIN_VERSION-x86_64-pc-windows-msvc;$env:PATH"
xwin --accept-license --manifest-version 16 splat --include-debug-symbols --include-debug-libs --use-winsysroot-style --preserve-ms-arch-notation --disable-symlinks --output "$conda_folder\xwin"

Write-Output("Cleaning Up")
Remove-Item "$conda_folder\xwin-$X_WIN_VERSION-x86_64-pc-windows-msvc" -ErrorAction SilentlyContinue -Force -Recurse
Remove-Item ".xwin-cache" -ErrorAction SilentlyContinue -Force -Recurse
Remove-Item "xwin.tar.gz" -ErrorAction SilentlyContinue -Force
