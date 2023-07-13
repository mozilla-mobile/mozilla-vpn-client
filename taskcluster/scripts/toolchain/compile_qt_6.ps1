# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"

$BIN_PATH = "$REPO_ROOT_PATH/bin"
$QT_VERSION = $env:QT_VERSION
$QT_VERSION_MAJOR = $QT_VERSION.split(".")[0..1] -join(".") # e.g 6.2.3 -> 6.2

Set-Location $FETCHES_PATH

Rename-Item -Path qt.zip.noextract -NewName qt.zip
unzip -o -qq qt.zip
unzip -o -qq open_ssl_win.zip # See toolchain/qt.yml for why

# Setup Openssl Import
$SSL_PATH = resolve-path "$FETCHES_PATH/SSL"
$env:OPENSSL_ROOT_DIR = (resolve-path "$SSL_PATH").toString()
$env:OPENSSL_USE_STATIC_LIBS = "TRUE"

Get-ChildItem env:
# Enter the DEV Shell
. "$FETCHES_PATH/VisualStudio/enter_dev_shell.ps1"

if(!(Test-Path $BIN_PATH)){
  New-Item -Path $BIN_PATH -ItemType "directory"
}
if(!(Test-Path $REPO_ROOT_PATH/QT_OUT)){
  New-Item -Path $REPO_ROOT_PATH/QT_OUT -ItemType "directory"
}

$BUILD_PREFIX = (resolve-path "$REPO_ROOT_PATH/QT_OUT").toString()

# Enter QT source directory
Set-Location $FETCHES_PATH/qt-everywhere-src-$QT_VERSION

./configure.bat `
  -static  `
  -opensource  `
  -debug-and-release `
  -no-dbus   `
  -no-feature-qdbus  `
  -confirm-license  `
  -strip  `
  -silent  `
  -nomake tests  `
  -nomake examples  `
  -make libs  `
  -no-sql-psql  `
  -no-sql-odbc   `
  -skip qt3d  `
  -skip webengine  `
  -skip qtmultimedia  `
  -skip qtserialport  `
  -skip qtsensors  `
  -skip qtgamepad  `
  -skip qtandroidextras  `
  -skip qtactiveqt  `
  -skip qtcharts  `
  -skip qtcoap  `
  -skip qtdatavis3d  `
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
  -skip qtwebview  `
  -skip qtlocation `
  -skip qtquick3dphysics `
  -feature-imageformat_png  `
  -qt-libpng  `
  -qt-zlib  `
  -openssl-runtime `
  -prefix $BUILD_PREFIX `

if($?){
    Write-Output "Qt configured! "
}else{
    Write-Output "Failed to configure!"
    exit 1
}

 cmake --build . --parallel

if($?){
  Write-Output "Qt BUILT "
}else{
  Write-Output "Failed to build!!"
  exit 1
}

 cmake --install . --config Debug
 cmake --install . --config Release


Set-Location $REPO_ROOT_PATH
Copy-Item -Path taskcluster/scripts/toolchain/configure_qt.ps1 -Destination QT_OUT/
Copy-Item -Path $SSL_PATH -Recurse -Destination QT_OUT/

New-Item -ItemType Directory -Path "$TASK_WORKDIR/public/build" -Force
zip -r "$TASK_WORKDIR/public/build/qt6_win.zip" QT_OUT


Write-Output "Build complete, zip created:"

# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue


Write-Output "Open Processes:"

wmic process get description,executablepath

