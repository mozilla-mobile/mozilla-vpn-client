# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$FETCHES_PATH =resolve-path "$REPO_ROOT_PATH/../../fetches"
$QTPATH =resolve-path "$FETCHES_PATH/QT_OUT/bin/"

# Prep Env:
# Enable qt, enable msvc, enable rust
. "$FETCHES_PATH/VisualStudio/enter_dev_shell.ps1"
. "$FETCHES_PATH/QT_OUT/configure_qt.ps1"
. "$REPO_ROOT_PATH/taskcluster/scripts/fetch/enable_win_rust.ps1"

# Fetch 3rdparty stuff.
python3 -m pip install -r requirements.txt --user
git submodule update --init --force --recursive --depth=1

# Fix: pip scripts are not on path by default on tc, so glean would fail
$PYTHON_SCRIPTS =resolve-path "$env:APPDATA\Python\Python36\Scripts"
$env:PATH ="$QTPATH;$PYTHON_SCRIPTS;$env:PATH"

# Set Env's required for the windows_compile.bat
$env:VCToolsRedistDir=(resolve-path "$FETCHES_PATH/VisualStudio/VC/Redist/MSVC/14.30.30704/").ToString()
$env:BUILDDIR=resolve-path $FETCHES_PATH/QT_OUT

# TODO: Remove this and change all to Microsoft_VC143 once we know there is no cavecat building with msvcv143
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x86.msm

# CMD does for some reason not use the new PATH, thus
# We need to pre-generate those resources here.
python3 ./scripts/utils/generate_glean.py
python3 ./scripts/utils/import_languages.py

./scripts/windows_compile.bat -w --nmake

Write-Output "Writing Artifacts"

New-Item -Path $REPO_ROOT_PATH/artifacts -ItemType "directory"
Copy-Item -Path windows/installer/x64/MozillaVPN.msi -Destination ./artifacts/MozillaVPN.msi

New-Item -Path $REPO_ROOT_PATH/unsigned -ItemType "directory"
Copy-Item -Path libssl-1_1-x64.dll -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path libcrypto-1_1-x64.dll -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path Microsoft_VC143_CRT_x64.msm -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path windows/tunnel/x64/tunnel.dll -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path windows/tunnel/.deps/wintun/bin/amd64/wintun.dll -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path balrog/x64/balrog.dll  -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path windows/split-tunnel/mullvad-split-tunnel.cat -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path windows/split-tunnel/mullvad-split-tunnel.inf -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path windows/split-tunnel/mullvad-split-tunnel.sys -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path windows/split-tunnel/WdfCoinstaller01011.dll -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path extension/manifests/windows/mozillavpn.json -Destination $REPO_ROOT_PATH/unsigned
Copy-Item -Path *.exe -Destination $REPO_ROOT_PATH/unsigned

Compress-Archive -Path unsigned/* -Destination $REPO_ROOT_PATH/artifacts/unsigned.zip

Write-Output "Artifacts Location: $REPO_ROOT_PATH/artifacts"
Get-ChildItem -Path $REPO_ROOT_PATH/artifacts


# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "vctip.exe" -Force -ErrorAction SilentlyContinue
