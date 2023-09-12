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
Set-Location $conda_folder
Write-Output("Downloading x-win")
# Small X-Win appretiation comment. 
# It's really great, 
# it can process windows store manifests, request files from MS-servers and unpack it. 
# we can use it to get the offical SDK files from microsoft. 
# We can't cargo-install it because to use the clang-msvc compat mode, 
# 
Invoke-WebRequest -Uri "https://github.com/Jake-Shadle/xwin/releases/download/$X_WIN_VERSION/xwin-$X_WIN_VERSION-x86_64-pc-windows-msvc.tar.gz" -OutFile "xwin.tar.gz"  
$ProgressPreference = 'Continue'
Write-Output("Unpack x-win")
tar -xf xwin.tar.gz
Copy-Item -Path "xwin-$X_WIN_VERSION-x86_64-pc-windows-msvc/xwin.exe"  -Destination "$conda_folder/bin"

Remove-Item "xwin-0.2.10-x86_64-pc-windows-msvc" -Confirm  -ErrorAction SilentlyContinue -Force -Recurse
Remove-Item "xwin.tar.gz" -Confirm -ErrorAction SilentlyContinue -Force -Recurse

$env:PATH ="$conda_folder\bin;$env:PATH"

# Splat the CRT and SDK files to /xwin/crt and /xwin/sdk respectively
Write-Output("Downloading the windows SDK")
xwin --accept-license --manifest-version 16 splat --include-debug-symbols --include-debug-libs --output xwin

## Generate the INCLUDE env
$XWIN_PATH="`$env:CONDA_PREFIX\xwin"
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

# Generate & write (de/)activation scripts


$activate = @"
`$env:CXX="clang-cl"
`$env:CC="clang-cl"
`$env:AR="llvm-lib"
`$env:LD="lld-link"

`$env:CMAKE_CXX_COMPILER="clang-cl"
`$env:CMAKE_C_COMPILER="clang-cl"
`$env:CMAKE_C_COMPILER="clang-cl"
`$env:CMAKE_LINKER="lld-link"
`$env:CMAKE_MT="llvm-mt"
`$env:CMAKE_GENERATOR="Ninja"

`$env:CC_x86_64_pc_windows_msvc="clang-cl"
`$env:CXX_x86_64_pc_windows_msvc="clang-cl"
`$env:AR_x86_64_pc_windows_msvc="llvm-lib"
`$env:LD_x86_64_pc_windows_msvc="lld-link"
`$env:CL_FLAGS="-Wno-unused-command-line-argument -fuse-ld=lld-link `$env:CONDA_PREFIX\\xwin\\crt\\include `$env:CONDA_PREFIX\\xwin\\sdk\\include\\ucrt `$env:CONDA_PREFIX\\xwin\\sdk\\include\\um `$env:CONDA_PREFIX\\xwin\\sdk\\include\\shared"
`$env:RUSTFLAGS="-C linker=lld-link -Lnative=\\xwin\\crt\\lib\\x86_64 -Lnative=`$env:CONDA_PREFIX\\xwin\\sdk\\lib\\um\\x86_64 -Lnative=`$env:CONDA_PREFIX\\xwin\\sdk\\lib\\ucrt\\x86_64"

# Conda/go does not ship an activate.ps1 -> therefore stuff is broken on powershell. 
`$env:GOROOT="`$env:CONDA_PREFIX\go"

`$env:INCLUDE="$INCLUDE_TARGET"
`$env:LIB="$XWIN_PATH\sdk\lib\ucrt\x86_64;$XWIN_PATH\sdk\lib\um\x86_64;$XWIN_PATH\crt\lib\x86_64;"
"@
Out-File -Encoding utf8 `
         -FilePath  "$conda_folder\etc\conda\activate.d\vpn_clang_cl.ps1"`
         -InputObject $activate 


$deactivate = @"
Remove-Item Env:\OPENSSL_ROOT_DIR
Remove-Item Env:\QT_HOST_PATH
Remove-Item Env:\CMAKE_PREFIX_PATH
Remove-Item Env:\CXX
Remove-Item Env:\CC
Remove-Item Env:\AR
Remove-Item Env:\LD
Remove-Item Env:\CMAKE_CXX_COMPILER
Remove-Item Env:\CMAKE_C_COMPILER
Remove-Item Env:\CMAKE_C_COMPILER
Remove-Item Env:\CMAKE_LINKER
Remove-Item Env:\CMAKE_MT
Remove-Item Env:\CMAKE_GENERATOR
Remove-Item Env:\CC_x86_64_pc_windows_msvc
Remove-Item Env:\CXX_x86_64_pc_windows_msvc
Remove-Item Env:\AR_x86_64_pc_windows_msvc
Remove-Item Env:\LD_x86_64_pc_windows_msvc
Remove-Item Env:\CL_FLAGS
Remove-Item Env:\RUSTFLAGS
Remove-Item Env:\GOROOT
Remove-Item Env:\INCLUDE
Remove-Item Env:\LIB

"@
Out-File -Encoding utf8 `
         -FilePath  "$conda_folder\etc\conda\deactivate.d\vpn_clang_cl.ps1"`
         -InputObject $deactivate 


Write-Output("You are SET! - Please re-activate your conda env to have stuff applied.")
Set-Location $OLD_PWD
Remove-Item ".xwin-cache" -Confirm -ErrorAction SilentlyContinue -Force -Recurse
