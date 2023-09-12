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
  -confirm-license  `
  -silent  `
  -make libs  `
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
  -no-feature-style-fusion `
  -no-feature-style-windows `
  -no-feature-style-windowsvista `
  -no-feature-quickcontrols2-material `
  -no-feature-quickcontrols2-macos `
  -no-feature-sql-sqlite `
  -no-feature-sql `
  -no-feature-textodfwriter `
  -no-feature-networklistmanager `
  -no-feature-dbus `
  -skip qt3d  `
  -skip qtdoc `
  -skip qtgrpc `
  -skip qtconnectivity `
  -skip qtquickeffectmaker `
  -skip qtquicktimeline `
  -skip qtwebengine  `
  -skip qtlocation `
  -skip qtmultimedia  `
  -skip qtserialport  `
  -skip qtsensors  `
  -skip qtgamepad  `
  -skip qtandroidextras  `
  -skip qtquick3dphysics `
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
  -skip qtweb `
  -feature-imageformat_png  `
  -qt-libpng  `
  -qt-zlib  `
  -openssl-runtime `
  -prefix $BUILD_PREFIX `


 cmake --build . --parallel

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

