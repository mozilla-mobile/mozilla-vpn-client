# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


$X_WIN_VERSION = "0.6.5"

$conda_env = conda info --json | ConvertFrom-Json
$conda_folder = $conda_env.active_prefix 


Write-Output("Installing in $conda_folder")
$OLD_PWD = $PWD # Backup that to go back once we done. 
Set-Location $conda_folder 

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

if( -not (Test-Path "$conda_folder/bin")){
    New-Item -ItemType Directory -Force -Path "$conda_folder/bin"
}

Copy-Item -Path "xwin-$X_WIN_VERSION-x86_64-pc-windows-msvc/xwin.exe"  -Destination "$conda_folder/bin"
$env:PATH ="$conda_folder\bin;$env:PATH"

# Splat the CRT and SDK files to /xwin/crt and /xwin/sdk respectively
Write-Output("Downloading the windows SDK")
xwin --accept-license `
    --manifest-version 16 `
    splat `
    --use-winsysroot-style `
    --preserve-ms-arch-notation `
    --include-debug-symbols `
    --include-debug-libs `
    --output xwin

## Generate the INCLUDE env
$XWIN_PATH="`$env:CONDA_PREFIX\xwin"
$LIB_ADDS =   `
                  "$XWIN_PATH\VC\Tools\MSVC\14.29.16.10\lib\x64;",`
                  "$XWIN_PATH\Windows Kits\10\lib\10.0.22000\ucrt\x64;"
$LIB_TARGET =""
ForEach-Object -InputObject $LIB_ADDS {
    $LIB_TARGET +=($XWIN_PATH+"\"+$_)
}
$LIB_TARGET= $LIB_TARGET-replace("; ",";")

# Generate & write (de/)activation scripts


$activate = @"
`$env:CXX="clang-cl"
`$env:CC="clang-cl"
`$env:AR="llvm-lib"
`$env:LD="lld-link"

`$env:LIB="$LIB_TARGET"

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
`$env:CFLAGS="/winsysroot $XWIN_PATH"
`$env:CXXFLAGS="/winsysroot $XWIN_PATH"
`$env:CL_FLAGS="-Wno-unused-command-line-argument -fuse-ld=lld-link"
`$env:RUSTFLAGS="-Clinker=`$env:CONDA_PREFIX/Library/bin/lld-link.exe"

"@
Out-File -Encoding utf8 `
         -FilePath  "$conda_folder\etc\conda\activate.d\vpn_clang_cl.ps1"`
         -InputObject $activate 

Copy-Item $conda_folder\Library\bin\llvm-mt.exe $conda_folder\Library\bin\mt.exe
Copy-Item $conda_folder\Library\bin\lld-link.exe $conda_folder\Library\bin\link.exe
Copy-Item $conda_folder\Library\bin\clang-cl.exe $conda_folder\Library\bin\cl.exe

$deactivate = @"
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
Remove-Item Env:\INCLUDE
Remove-Item Env:\LIB

"@
Out-File -Encoding utf8 `
         -FilePath  "$conda_folder\etc\conda\deactivate.d\vpn_clang_cl.ps1"`
         -InputObject $deactivate 


Remove-Item ".xwin-cache" -Confirm -ErrorAction SilentlyContinue -Force -Recurse
Write-Output("You are SET! - Please re-activate your conda env to have stuff applied.")
Set-Location $OLD_PWD
