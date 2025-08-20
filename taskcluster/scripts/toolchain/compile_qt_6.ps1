# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

Set-Location $env:TASK_WORKDIR
Get-ChildItem env:

$QT_SRC_ARCHIVE =resolve-path "$env:MOZ_FETCHES_DIR/qt-everywhere-src-*.zip"

unzip -o -qq -d "$env:TASK_WORKDIR" $QT_SRC_ARCHIVE

# Activate the visual studio developer shell.
$VS_SHELL_HELPER = resolve-path "$env:MOZ_FETCHES_DIR/*/enter_dev_shell.ps1"
. "$VS_SHELL_HELPER"

$QT_SRC_PATH = (resolve-path "$env:TASK_WORKDIR/qt-everywhere-src-*/configure.bat" | Split-Path -Parent)
$QT_BUILD_PATH = "$env:TASK_WORKDIR/qt-build"
$QT_INSTALL_PATH = "$env:TASK_WORKDIR/qt-windows"
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
Start-Process -WorkingDirectory "$QT_BUILD_PATH" -NoNewWindow -Wait "$QT_SRC_PATH\configure.bat" -ArgumentList @(
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
)

# Build and install Qt
cmake --build $QT_BUILD_PATH --parallel
cmake --install $QT_BUILD_PATH --config Debug
cmake --install $QT_BUILD_PATH --config Release

Set-Location $env:TASK_WORKDIR
tar -cJf qt6_win.tar.xz qt-windows/

New-Item -ItemType Directory -Path "$env:TASK_WORKDIR/public/build" -Force
Move-Item -Path qt6_win.tar.xz -Destination "$env:TASK_WORKDIR/public/build"

Write-Output "Build complete, tarball created:"

# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue


Write-Output "Open Processes:"

wmic process get description,executablepath

