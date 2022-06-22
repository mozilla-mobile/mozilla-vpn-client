# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$FETCHES_PATH =resolve-path "$REPO_ROOT_PATH/../../fetches"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$QTPATH =resolve-path "$FETCHES_PATH/QT_OUT/bin/"

# Prep Env:
# Enable qt, enable msvc, enable rust
. "$FETCHES_PATH/VisualStudio/enter_dev_shell.ps1"
. "$FETCHES_PATH/QT_OUT/configure_qt.ps1"
. "$REPO_ROOT_PATH/taskcluster/scripts/fetch/enable_win_rust.ps1"

# Remove Long lasting ms-compiler-telemetry service: 
# This will sometimes live longer then our compile
# and __sometimes__ taskcluster will fail to do cleanup once the task is done
Remove-Item $FETCHES_PATH/VisualStudio/VC/Tools/MSVC/14.30.30705/bin/HostX64/x64/VCTIP.EXE  


# Fetch 3rdparty stuff.
python3 -m pip install -r requirements.txt --user
git submodule update --init --force --recursive --depth=1

# Fix: pip scripts are not on path by default on tc, so glean would fail
$PYTHON_SCRIPTS =resolve-path "$env:APPDATA\Python\Python36\Scripts"
$env:PATH ="$QTPATH;$PYTHON_SCRIPTS;$env:PATH"

# Set Env's required for the windows/compile.bat
$env:VCToolsRedistDir=(resolve-path "$FETCHES_PATH/VisualStudio/VC/Redist/MSVC/14.30.30704/").ToString()
$env:BUILDDIR=resolve-path $FETCHES_PATH/QT_OUT

# TODO: Remove this and change all to Microsoft_VC143 once we know there is no cavecat building with msvcv143
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x86.msm

# CMD does for some reason not use the new PATH, thus
# We need to pre-generate those resources here.
python3 ./scripts/utils/generate_glean.py
python3 ./scripts/utils/import_languages.py

#Do not continune from this point on when we encounter an error
$ErrorActionPreference = "Stop"

# Actually compile!
./scripts/windows/compile.bat --nmake
# Copies all relevant files into unsigned/
nmake install 

# For some reason qmake does ignore split-tunnel stuff
# But we are switching to cmake, which handles this fine
# so consider this a temporary fix :) 
Copy-Item -Path windows/split-tunnel/* -Destination unsigned -Exclude "*.ps1","*.txt",".status"

New-Item -ItemType Directory -Path "$TASK_WORKDIR/artifacts" -Force
$ARTIFACTS_PATH =resolve-path "$TASK_WORKDIR/artifacts"

Write-Output "Writing Artifacts"
Copy-Item -Path windows/installer/x64/MozillaVPN.msi -Destination $ARTIFACTS_PATH/MozillaVPN.msi
Copy-Item -Path MozillaVPN.pdb -Destination $ARTIFACTS_PATH/MozillaVPN.pdb

Compress-Archive -Path unsigned/* -Destination $TASK_WORKDIR/artifacts/unsigned.zip

Write-Output "Artifacts Location:$TASK_WORKDIR/artifacts"
Get-ChildItem -Path $TASK_WORKDIR/artifacts


# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "vctip.exe" -Force -ErrorAction SilentlyContinue

Write-Output "Open Processes:"

wmic process get description,executablepath 
