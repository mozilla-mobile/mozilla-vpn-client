# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

Set-Location $env:TASK_WORKDIR
Get-ChildItem env:

# Extract the Qt source tarball.
$QT_SRC_FILENAME = (resolve-path "$env:MOZ_FETCHES_DIR/qt-everywhere-src-*.tar.xz" | Split-Path -Leaf)
Start-Process -WorkingDirectory "$env:MOZ_FETCHES_DIR" -NoNewWindow -Wait "tar" -ArgumentList @('xf', "$QT_SRC_FILENAME")

# Activate the visual studio developer shell.
$VS_SHELL_HELPER = resolve-path "$env:MOZ_FETCHES_DIR/*/enter_dev_shell.ps1"
. "$VS_SHELL_HELPER"

$QT_CONFIG_SCRIPT = resolve-path "$env:MOZ_FETCHES_DIR/qt-everywhere-src-*/configure.bat"
$QT_BUILD_PATH = "$env:TASK_WORKDIR\qt-build"
$QT_INSTALL_PATH = "$env:TASK_WORKDIR\qt-windows"
if(!(Test-Path $QT_INSTALL_PATH)){
  New-Item -Path qt-windows -ItemType "directory"
}
if(!(Test-Path $QT_BUILD_PATH)){
  New-Item -Path qt-build -ItemType "directory"
}
Copy-Item -Path "$env:VCS_PATH/taskcluster/scripts/toolchain/configure_qt.ps1" -Destination qt-windows/

# Setup Openssl Import
unzip -o -qq -d "$env:TASK_WORKDIR/qt-windows" "$env:MOZ_FETCHES_DIR/open_ssl_win.zip" # See toolchain/qt.yml for why
$SSL_PATH = "$env:TASK_WORKDIR/qt-windows/SSL"
if (Test-Path -Path $SSL_PATH) {
  $env:OPENSSL_ROOT_DIR = (resolve-path "$SSL_PATH").toString()
  $env:OPENSSL_USE_STATIC_LIBS = "TRUE"
}

$ErrorActionPreference = "Stop"

# Let's trim what we dont need.
# See for general config: https://github.com/qt/qtbase/blob/dev/config_help.txt
# For detailed feature flags, run the configuration, then check the CMakeLists.txt
# Variables with FEATURE_XYZ can be switched off using -no-feature
# Whole folders can be skipped using -skip <folder>
Start-Process -WorkingDirectory "$QT_BUILD_PATH" -NoNewWindow -PassThru $QT_CONFIG_SCRIPT -ArgumentList @(
  '-static'
  '-opensource'
  '-debug-and-release'
  '-confirm-license'
  '-silent'
  '-make libs'
  '-nomake tests'
  '-nomake examples'
  '-no-feature-dynamicgl'
  '-no-feature-sql-odbc'
  '-no-feature-pixeltool'
  '-no-feature-qdbus'
  '-no-feature-qtattributionsscanner'
  '-no-feature-qtdiag'
  '-no-feature-qtplugininfo'
  '-no-feature-pixeltool'
  '-no-feature-distancefieldgenerator'
  '-no-feature-designer'
  '-no-feature-assistant'
  '-no-feature-sql-sqlite'
  '-no-feature-sql'
  '-no-feature-textodfwriter'
  '-no-feature-networklistmanager'
  '-no-feature-dbus'
  '-skip qtgraphs'
  '-skip qt3d'
  '-skip qtdoc'
  '-skip qtgrpc'
  '-skip qtconnectivity'
  '-skip qtquickeffectmaker'
  '-skip qtwebengine'
  '-skip qtwebview'
  '-skip qtlocation'
  '-skip qtserialport'
  '-skip qtsensors'
  '-skip qtgamepad'
  '-skip qtandroidextras'
  '-skip qtquick3dphysics'
  '-skip qtactiveqt'
  '-skip qtcoap'
  '-skip qtgrpc'
  '-skip qtremoteobjects'
  '-skip qtlottie'
  '-skip qtmqtt'
  '-skip qtopcua'
  '-skip qtpositioning'
  '-skip qtquick3d'
  '-skip qtscxml'
  '-skip qtserialbus'
  '-skip qtserialport'
  '-skip qtspeech'
  '-skip qtwayland'
  '-skip qtvirtualkeyboard'
  '-skip qtweb'
  '-feature-imageformat_png'
  '-qt-libpng'
  '-qt-zlib'
  "-prefix $QT_INSTALL_PATH"
) | Wait-Process

# Build and install Qt
Write-Output "Starting build: $QT_BUILD_PATH"
cmake --build $QT_BUILD_PATH --parallel --verbose
if ($LastExitCode -ne 0) {
  Exit $LastExitCode
}

cmake --install $QT_BUILD_PATH --config Debug
if ($LastExitCode -ne 0) {
  Exit $LastExitCode
}

cmake --install $QT_BUILD_PATH --config Release
if ($LastExitCode -ne 0) {
  Exit $LastExitCode
}

tar -cJf qt6_win.tar.xz qt-windows/
if ($LastExitCode -ne 0) {
  Exit $LastExitCode
}

New-Item -ItemType Directory -Path "$env:TASK_WORKDIR/public/build" -Force
Move-Item -Path qt6_win.tar.xz -Destination "$env:TASK_WORKDIR/public/build"

Write-Output "Build complete, tarball created:"

# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue
