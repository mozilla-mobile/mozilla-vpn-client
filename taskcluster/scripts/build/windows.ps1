# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"
$QTPATH =resolve-path "$FETCHES_PATH/QT_OUT/bin/"
$PERL_GCC_PATH =resolve-path "$FETCHES_PATH/c/bin"
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

# Setup Go and MinGW up (for gco)
$env:GOROOT="$FETCHES_PATH\go\"
$env:PATH ="$FETCHES_PATH\go\bin;$env:PATH"
$env:PATH = "$env:PATH;$PERL_GCC_PATH;"

# Set Env's required for the windows/compile.bat
$env:VCToolsRedistDir=(resolve-path "$FETCHES_PATH/VisualStudio/VC/Redist/MSVC/14.30.30704/").ToString()
$env:BUILDDIR=resolve-path $FETCHES_PATH/QT_OUT

# TODO: Remove this and change all to Microsoft_VC143 once we know there is no cavecat building with msvcv143
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $REPO_ROOT_PATH\\Microsoft_VC142_CRT_x86.msm

Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x64.msm -Destination $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC142_CRT_x64.msm
Copy-Item -Path $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC143_CRT_x86.msm -Destination $env:VCToolsRedistDir\\MergeModules\\Microsoft_VC142_CRT_x86.msm


# Setup Openssl Import
$SSL_PATH = resolve-path "$FETCHES_PATH/QT_OUT/SSL"
$env:OPENSSL_ROOT_DIR = (resolve-path "$SSL_PATH").toString()
$env:OPENSSL_USE_STATIC_LIBS = "TRUE"

#Do not continune from this point on when we encounter an error
$ErrorActionPreference = "Stop"
mkdir $TASK_WORKDIR/cmake_build
$BUILD_DIR =resolve-path "$TASK_WORKDIR/cmake_build"


cmake --version
cmake -S . -B $BUILD_DIR -GNinja -DCMAKE_BUILD_TYPE=Release # TODO: Linking breaks horribly with RelWithDebInfo
cmake --build $BUILD_DIR #--config RelWithDebInfo Ignored as we are using ninja
cmake --build $BUILD_DIR --target msi
cmake --install $BUILD_DIR --prefix "$TASK_WORKDIR/unsigned"

Write-Output "Writing Artifacts"
New-Item -ItemType Directory -Path "$TASK_WORKDIR/artifacts" -Force
$ARTIFACTS_PATH =resolve-path "$TASK_WORKDIR/artifacts"
Copy-Item -Path $BUILD_DIR/windows/installer/MozillaVPN.msi -Destination $ARTIFACTS_PATH/MozillaVPN.msi
# Note: vc140.pdb is just the default name for pdb files (as we are using vc14x)
Copy-Item -Path "$BUILD_DIR/src/CMakeFiles/mozillavpn.dir/vc140.pdb" -Destination "$ARTIFACTS_PATH/Mozilla VPN.pdb"
Compress-Archive -Path $TASK_WORKDIR/unsigned/* -Destination $ARTIFACTS_PATH/unsigned.zip
Write-Output "Artifacts Location:$TASK_WORKDIR/artifacts"
Get-ChildItem -Path $TASK_WORKDIR/artifacts

# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "vctip.exe" -Force -ErrorAction SilentlyContinue

Write-Output "Open Processes:"

wmic process get description,executablepath
