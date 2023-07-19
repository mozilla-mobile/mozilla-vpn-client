# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$TASK_WORKDIR =resolve-path "$REPO_ROOT_PATH/../../"
$FETCHES_PATH =resolve-path "$TASK_WORKDIR/fetches"

$BIN_PATH = "$REPO_ROOT_PATH/bin"
$QT_VERSION = $env:QT_VERSION
$QT_VERSION_MAJOR = $QT_VERSION.split(".")[0..1] -join(".") # e.g 6.2.3 -> 6.2

# Init conda, download curl. 
# Powerhshell's native downloading is ... slow. 
. $PSScriptRoot/conda_init.ps1
conda install -y  curl
Get-Command curl

# Let's use the taskcluster one for a bit. 
$QT_URI = "https://download.qt.io/archive/qt/$QT_VERSION_MAJOR/$QT_VERSION/single/qt-everywhere-src-$QT_VERSION.zip"

Set-Location $FETCHES_PATH
Write-Output "Downloading : $QT_URI"
conda run curl $QT_URI -O qt-everywhere-src-$QT_VERSION.zip
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


##Fix Line endings of qsb files
Get-ChildItem * -Include *.qsb | ForEach-Object {
  ## If contains UNIX line endings, replace with Windows line endings
  if (Get-Content $_.FullName -Delimiter "`0" | Select-String "[^`r]`n")
  {
      $content = Get-Content $_.FullName
      $content | Set-Content $_.FullName
  }
}


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
  -skip qtwebchannel  `
  -skip qtandroidextras  `
  -skip qtquick3dphysics `
  -feature-imageformat_png  `
  -no-feature-quick-shadereffect `
  -qt-libpng  `
  -qt-zlib  `
  -openssl-runtime `
  -prefix $BUILD_PREFIX `

# Print all the targets, so i can build the problematic ones first, 
# save some time. 
#cmake --build . --target help

 cmake --build . --parallel

 if($?){
  Write-Output "Compiled QT"
}else{
  Write-Output "Failed to Compile QT"
   cmake --build .  --verbose
  # So this did indeed not output anything....
   ./qtbase/bin/qsb.exe --glsl 100es,120,150 --hlsl 50 --msl 12 -b -c -O -s -o qtdeclarative/src/quick/.qsb/scenegraph/shaders_ng/8bittextmask_a.frag.qsb qtdeclarative/src/quick/scenegraph/shaders_ng/8bittextmask_a.frag
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

