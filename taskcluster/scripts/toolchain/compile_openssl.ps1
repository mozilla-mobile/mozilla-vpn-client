# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"

# Enter the DEV Shell
. "$FETCHES_PATH/VisualStudio/enter_dev_shell.ps1"

#New-Item -Path fetches -ItemType "directory"
Set-Location -Path $FETCHES_PATH

Invoke-WebRequest -Uri https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1m.zip -OutFile openssl.zip
Invoke-WebRequest -Uri https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-win64.zip -OutFile nasm.zip

Expand-Archive nasm.zip -DestinationPath .
Expand-Archive openssl.zip -DestinationPath .

Set-Location openssl-OpenSSL_1_1_1m

$NASM_PATH = resolve-path ../nasm-2.15.05
$env:PATH = "$env:PATH;$NASM_PATH"

if(!(Test-Path ../SSL)){
    New-Item -Path ../SSL -ItemType "directory"
}
# Means we put it $FETCHES_PATH/SSL
$SSL_OUT_PATH = resolve-path ../SSL

$PERL_PATH =resolve-path "$FETCHES_PATH/perl/bin"
$env:PATH = "$PERL_PATH;$env:PATH"

perl Configure  VC-WIN64A `
                --release `
                --prefix="$SSL_OUT_PATH" `
                --openssldir="$SSL_OUT_PATH/SSL" `
                no-tests `
                no-engine `
                -DOPENSSL_NO_ENGINE

$env:CL="/MP"

nmake build_libs
nmake install
Set-Location $FETCHES_PATH

New-Item -ItemType Directory -Path "$TASK_WORKDIR/public/build" -Force
Compress-Archive -Path $SSL_OUT_PATH -DestinationPath "$TASK_WORKDIR/public/build/open_ssl_win.zip"
