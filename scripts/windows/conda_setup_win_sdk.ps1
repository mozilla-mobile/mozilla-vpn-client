# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


$X_WIN_VERSION = "0.2.10"

$conda_env = conda info --json | ConvertFrom-Json


if( $conda_env.active_prefix_name -eq "base"){
    Write-Output("Not in an active conda env. abort")
    return -1
}

Write-Output("Downloading x-win")
# Small X-Win appretiation comment. 
# It's really great, 
# it can process windows store manifests, request files from MS-servers and unpack it. 
# we can use it to get the offical SDK files from microsoft. 
# We can't cargo-install it because to use the clang-msvc compat mode, 
# 
Invoke-WebRequest -Uri "https://github.com/Jake-Shadle/xwin/releases/download/$X_WIN_VERSION/xwin-$X_WIN_VERSION-x86_64-pc-windows-msvc.tar.gz" -OutFile "xwin.tar.gz"  
$ProgressPreference = 'Continue'
tar -xf xwin.tar.gz
Copy-Item -Path "xwin-$X_WIN_VERSION-x86_64-pc-windows-msvc/xwin.exe"  -Destination "$conda_folder/bin"
Remove-Item "xwin-0.2.10-x86_64-pc-windows-msvc" -Confirm  -ErrorAction SilentlyContinue -Force
Remove-Item "xwin.tar.gz" -Confirm -ErrorAction SilentlyContinue -Force

# Splat the CRT and SDK files to /xwin/crt and /xwin/sdk respectively
Write-Output("Downloading the windows SDK")
xwin --accept-license --manifest-version 16 splat --include-debug-symbols --include-debug-libs --output xwin

Write-Output("Updating the current Conda ENV")
# Set rust variables for target. 
conda env config vars set CC_x86_64_pc_windows_msvc="clang-cl" | Out-Null
conda env config vars set CXX_x86_64_pc_windows_msvc="clang-cl" | Out-Null
conda env config vars set AR_x86_64_pc_windows_msvc="llvm-lib"| Out-Null
conda env config vars set LD_x86_64_pc_windows_msvc="lld-link"| Out-Null
# Make sure clang can find stuff
conda env config vars set CL_FLAGS="-Wno-unused-command-line-argument -fuse-ld=lld-link $conda_folder\\xwin\\crt\\include $conda_folder\\xwin\\sdk\\include\\ucrt $conda_folder\\xwin\\sdk\\include\\um $conda_folder\\xwin\\sdk\\include\\shared"  | Out-Null
conda env config vars set RUSTFLAGS="-Lnative=\\xwin\\crt\\lib\\x86_64 -Lnative=$conda_folder\\xwin\\sdk\\lib\\um\\x86_64 -Lnative=$conda_folder\\xwin\\sdk\\lib\\ucrt\\x86_64" | Out-Null

# Overwrite default conf. 
conda env config vars set CXX="clang-cl" | Out-Null
conda env config vars set CC="clang-cl" | Out-Null
conda env config vars set AR="llvm-lib"| Out-Null
conda env config vars set LD="lld-link"| Out-Null
conda env config vars set CMAKE_CXX_COMPILER="clang-cl" | Out-Null
conda env config vars set CMAKE_C_COMPILER="clang-cl" | Out-Null
conda env config vars set CMAKE_C_COMPILER="clang-cl" | Out-Null
conda env config vars set CMAKE_GENERATOR="Ninja" | Out-Null

# TODO: Check why Go-Root is broken on conda-forge. 
# it's pointing to D:\bld\go-cgo_1670946228663\work\go
# Probably a CI related path? 
conda env config vars set GOROOT="$conda_folder\go"| Out-Null

$XWIN_PATH="$conda_folder\xwin"

$INCLUDE_ADDS =   `
                  "$XWIN_PATH\sdk\include;",`
                  "$XWIN_PATH\crt\include;",`
                  "$XWIN_PATH\sdk\include\cppwinrt\winrt;",`
                  "$XWIN_PATH\sdk\include\shared;",`
                  "$XWIN_PATH\sdk\include\ucrt;" ,`
                  "$XWIN_PATH\sdk\include\um;",`
                  "$XWIN_PATH\sdk\include\winrt"
$INCLUDE_TARGET =""
ForEach-Object -InputObject $INCLUDE_ADDS {
    $INCLUDE_TARGET +=($XWIN_PATH+"\"+$_)
}
$INCLUDE_TARGET= $INCLUDE_TARGET-replace("; ",";")

conda env config vars set INCLUDE=$INCLUDE_TARGET | Out-Null
conda env config vars set LIB="$XWIN_PATH\sdk\lib\ucrt\x86_64;$XWIN_PATH\sdk\lib\um\x86_64;$XWIN_PATH\crt\lib\x86_64;" | Out-Null
# Leaving this here: 
# It's set in the MSVC dev enviroment but it seems we're fine without it. 
#conda env config vars set LIBPATH="$XWIN_PATH\sdk\lib\ucrt\x86_64;$XWIN_PATH\sdk\lib\um\x86_64;$XWIN_PATH\crt\lib\x86_64;" | Out-Null

Write-Output("You are SET! - Please re-activate your conda env to have stuff applied.")
Set-Location $OLD_PWD
