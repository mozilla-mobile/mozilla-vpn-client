# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

Set-Location $env:TASK_WORKDIR
Get-ChildItem env:

# Define the submodules to initialize
$QT_SUBMODULES = @(
    'qt5compat'
    'qtactiveqt'
    'qtbase'
    'qtdeclarative'
    'qtimageformats'
    'qtlanguageserver'
    'qtnetworkauth'
    'qtrepotools'
    'qtwebsockets'
    'qtshadertools'
    'qttools'
    'qtsvg'
)

# Activate the visual studio developer shell.
$VS_SHELL_HELPER = resolve-path "$env:MOZ_FETCHES_DIR/*/enter_dev_shell.ps1"
. "$VS_SHELL_HELPER"

$QT_BUILD_PATH = "$env:TASK_WORKDIR\qt-build"
$QT_INSTALL_PATH = "$env:TASK_WORKDIR\qt-windows"

git clone --branch $env:QT_VERSION --depth 1 --single-branch git://code.qt.io/qt/qt5.git qt
Set-Location qt

git submodule init $QT_SUBMODULES
git submodule update --depth 1 --recursive

# Note: QtTools has more submoudles!
git submodule update --init --recursive --depth 1 --shallow-submodules qttools

$ErrorActionPreference = "Stop"

cmake -S . -B $QT_BUILD_PATH `
    -DFEATURE_relocatable=ON `
    -DQT_FEATURE_debug_and_release=ON `
    -DQT_BUILD_TESTS=OFF `
    -DBUILD_SHARED_LIBS=OFF `
    -DFEATURE_developer_build=OFF `
    -DFEATURE_assistant=OFF `
    -DFEATURE_designer=OFF `
    -DFEATURE_qtdiag=OFF `
    -DFEATURE_sql=OFF 

# Build and install Qt
Write-Output "Starting build: $QT_BUILD_PATH"
cmake --build $QT_BUILD_PATH --parallel --verbose
if ($LastExitCode -ne 0) {
  Exit $LastExitCode
}

Write-Output "Installing debug Qt:"
cmake --install $QT_BUILD_PATH --config Debug
if ($LastExitCode -ne 0) {
  Exit $LastExitCode
}

Write-Output "Installing release Qt:"
cmake --install $QT_BUILD_PATH --config Release
if ($LastExitCode -ne 0) {
  Exit $LastExitCode
}

Write-Output "Compressing tarball"
New-Item -ItemType Directory -Path "$env:TASK_WORKDIR/public/build" -Force
tar -cvJf public/build/qt6_win.tar.xz qt-windows/
if ($LastExitCode -ne 0) {
  Exit $LastExitCode
}

Write-Output "Build complete, tarball created:"

# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue
