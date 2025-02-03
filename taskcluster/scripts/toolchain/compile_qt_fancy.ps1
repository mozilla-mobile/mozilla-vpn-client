# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. "$PSScriptRoot/../common/helpers.ps1"

$ErrorActionPreference = "Stop"
# Unpack Conda-Base
Install-MiniConda
ls $FETCHES_PATH
$CONDA_DIR = resolve-path "$FETCHES_PATH\"
Start-Process -NoNewWindow "$CONDA_DIR\Scripts\conda-unpack.exe" -Wait
conda activate $CONDA_DIR
conda info

# Enter the DEV Shell
. "$FETCHES_PATH/msvc/enter_dev_shell.ps1" -arch arm64
Get-ChildItem env:

# Clone QT Code
git clone --branch $env:QT_VERSION --depth 1 --single-branch git://code.qt.io/qt/qt5.git qt
Set-Location qt 
git submodule init qt5compat qtactiveqt qtbase qtdeclarative qtimageformats qtlanguageserver qtnetworkauth qtrepotools qtwebsockets qtshadertools qttools qtsvg
git submodule update --depth 1 --recursive

# Note: QtTools has more submoudles!
git submodule update --init --recursive --depth 1 --shallow-submodules qttools

# Setup Output Dirs
if(!(Test-Path $REPO_ROOT_PATH/QT_OUT)){
  New-Item -Path $REPO_ROOT_PATH/QT_OUT -ItemType "directory"
}
$BUILD_PREFIX = (resolve-path "$REPO_ROOT_PATH/QT_OUT").toString()

$ErrorActionPreference = "Stop"
  # For newer qt versions, let's trim what we dont need.
  # See for general config: https://github.com/qt/qtbase/blob/dev/config_help.txt
  # For detailed feature flags, run the configuration, then check the CMakeLists.txt
  # Variables with FEATURE_XYZ can be switched off using -no-feature
  # Whole folders can be skipped using -skip <folder>

cmake -S . -B build `
    -DFEATURE_relocatable=ON `
    -DFEATURE_developer_build=OFF `
    -DQT_FEATURE_assistant=OFF `
    -DQT_FEATURE_designer=OFF `
    -DQT_FEATURE_qtdiag=OFF `
    -DQT_FEATURE_debug_and_release=ON `
    -DBUILD_SHARED_LIBS=OFF `
    -DQT_BUILD_TESTS=OFF `

 cmake --build . --parallel

 cmake --install . --config Debug --prefix $BUILD_PREFIX
 cmake --install . --config Release --prefix $BUILD_PREFIX

Set-Location $REPO_ROOT_PATH
Copy-Item -Path taskcluster/scripts/toolchain/configure_qt.ps1 -Destination QT_OUT/



New-Item -ItemType Directory -Path "$TASK_WORKDIR/public/build" -Force
zip -r "$TASK_WORKDIR/public/build/qt6_win.zip" QT_OUT


Write-Output "Build complete, zip created:"

# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue


Write-Output "Open Processes:"

wmic process get description,executablepath

