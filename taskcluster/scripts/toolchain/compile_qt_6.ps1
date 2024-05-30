# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"

$BIN_PATH = "$REPO_ROOT_PATH/bin"
$QT_VERSION = $env:QT_VERSION
$QT_VERSION_MAJOR = $QT_VERSION.split(".")[0..1] -join(".") # e.g 6.2.3 -> 6.2

$QT_URI = "https://download.qt.io/archive/qt/$QT_VERSION_MAJOR/$QT_VERSION/single/qt-everywhere-src-$QT_VERSION.zip"

Set-Location $FETCHES_PATH
Write-Output "Downloading : $QT_URI"
Invoke-WebRequest -Uri $QT_URI -OutFile qt-everywhere-src-$QT_VERSION.zip
if($?){
    Write-Output "Downloaded : $QT_URI"
}else{
    Write-Output "Failed to download : $QT_URI"
    exit 1
}

unzip -o -qq qt-everywhere-src-$QT_VERSION.zip
unzip -o -qq open_ssl_win.zip # See toolchain/qt.yml for why

# Setup Openssl Import

# Setup Openssl Import
$SSL_PATH = "$FETCHES_PATH/SSL"
if (Test-Path -Path $SSL_PATH) {
  $env:OPENSSL_ROOT_DIR = (resolve-path "$SSL_PATH").toString()
  $env:OPENSSL_USE_STATIC_LIBS = "TRUE"
}

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

$ErrorActionPreference = "Stop"

if($QT_VERSION_MAJOR -eq "6.2" ){
  # We should not chane the behavior mid release. 
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
  -qt-sqlite  `
  -skip qt3d  `
  -skip webengine  `
  -skip qtmultimedia  `
  -skip qtserialport  `
  -skip qtsensors  `
  -skip qtgamepad  `
  -skip qtwebchannel  `
  -skip qtandroidextras  `
  -feature-imageformat_png  `
  -qt-libpng  `
  -qt-zlib  `
  -openssl-runtime `
  -prefix $BUILD_PREFIX `
} else {
  # For newer qt versions, let's trim what we dont need.
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
} 





 cmake --build . --parallel

 cmake --install . --config Debug
 cmake --install . --config Release


Set-Location $REPO_ROOT_PATH
Copy-Item -Path taskcluster/scripts/toolchain/configure_qt.ps1 -Destination QT_OUT/

if (Test-Path -Path $SSL_PATH) {
  Copy-Item -Path $SSL_PATH -Recurse -Destination QT_OUT/
}

New-Item -ItemType Directory -Path "$TASK_WORKDIR/public/build" -Force
zip -r "$TASK_WORKDIR/public/build/qt6_win.zip" QT_OUT


Write-Output "Build complete, zip created:"

# mspdbsrv might be stil running after the build, so we need to kill it
Stop-Process -Name "mspdbsrv.exe" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "mspdbsrv" -Force -ErrorAction SilentlyContinue


Write-Output "Open Processes:"

wmic process get description,executablepath

