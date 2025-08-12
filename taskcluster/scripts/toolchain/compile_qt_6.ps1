# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

Set-Location $env:TASK_WORKDIR
$QT_SRC_ARCHIVE =resolve-path "$env:MOZ_FETCHES_DIR/qt-everywhere-src-*.zip"

unzip -o -qq -d "$env:MOZ_FETCHES_DIR" $QT_SRC_ARCHIVE

Get-ChildItem env:
# Enter the DEV Shell
. "$env:MOZ_FETCHES_DIR/VisualStudio/enter_dev_shell.ps1"

$QT_INSTALL_PATH = "$env:TASK_WORKDIR/qt-windows"
if(!(Test-Path qt-windows)){
  New-Item -Path qt-windows -ItemType "directory"
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

$BUILD_PREFIX = (resolve-path "$env:TASK_WORKDIR/qt-windows").toString()
$QT_SRC_PATH = (resolve-path "$env:MOZ_FETCHES_DIR/qt-everywhere-src-*/configure.bat" | Split-Path -Parent)
Set-Location $QT_SRC_PATH

# Let's trim what we dont need.
# See for general config: https://github.com/qt/qtbase/blob/dev/config_help.txt
# For detailed feature flags, run the configuration, then check the CMakeLists.txt
# Variables with FEATURE_XYZ can be switched off using -no-feature
# Whole folders can be skipped using -skip <folder>
./configure.bat `
  -static  `
  -opensource  `
  -debug-and-release `
  -confirm-license  `
  -silent  `
  -make libs  `
  -nomake tests  `
  -nomake examples  `
  -no-feature-dynamicgl `
  -no-feature-sql-odbc `
  -no-feature-pixeltool `
  -no-feature-qdbus `
  -no-feature-qtattributionsscanner `
  -no-feature-qtdiag `
  -no-feature-qtplugininfo `
  -no-feature-pixeltool `
  -no-feature-distancefieldgenerator `
  -no-feature-designer `
  -no-feature-assistant `
  -no-feature-sql-sqlite `
  -no-feature-sql `
  -no-feature-textodfwriter `
  -no-feature-networklistmanager `
  -no-feature-dbus `
  -skip qtgraphs `
  -skip qt3d  `
  -skip qtdoc `
  -skip qtgrpc `
  -skip qtconnectivity `
  -skip qtquickeffectmaker `
  -skip qtwebengine  `
  -skip qtlocation `
  -skip qtserialport  `
  -skip qtsensors  `
  -skip qtgamepad  `
  -skip qtandroidextras  `
  -skip qtquick3dphysics `
  -skip qtactiveqt  `
  -skip qtcoap  `
  -skip qtgrpc  `
  -skip qtremoteobjects  `
  -skip qtlottie  `
  -skip qtmqtt  `
  -skip qtopcua  `
  -skip qtpositioning  `
  -skip qtquick3d  `
  -skip qtscxml  `
  -skip qtserialbus  `
  -skip qtserialport  `
  -skip qtspeech  `
  -skip qtwayland  `
  -skip qtvirtualkeyboard  `
  -skip qtweb `
  -feature-imageformat_png  `
  -qt-libpng  `
  -qt-zlib  `
  -prefix $BUILD_PREFIX `

# Build and install Qt
cmake --build . --parallel
cmake --install . --config Debug
cmake --install . --config Release

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

