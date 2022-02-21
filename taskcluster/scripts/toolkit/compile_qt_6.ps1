$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$FETCHES_PATH =resolve-path "$REPO_ROOT_PATH/../../fetches"


$BIN_PATH = "$REPO_ROOT_PATH/bin"

Set-Location $FETCHES_PATH 
Invoke-WebRequest -Uri https://download.qt.io/archive/qt/6.2/6.2.3/single/qt-everywhere-src-6.2.3.zip -OutFile qt-everywhere-src-6.2.3.zip
unzip -o -qq qt-everywhere-src-6.2.3.zip
unzip -o -qq open_ssl_win.zip # See Build-qt/windows.yml why

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
Set-Location $FETCHES_PATH/qt-everywhere-src-6.2.3

./configure.bat `
  -static  `
  -opensource  `
  -release  `
  -no-dbus   `
  -no-feature-qdbus  `
  -confirm-license  `
  -strip  `
  -silent  `
  -nomake tests  `
  -nomake examples  `
  -make libs  `
  -no-sql-psql  `
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

  

 cmake --build . --parallel

 cmake --install .


Set-Location $REPO_ROOT_PATH
Copy-Item -Path taskcluster/scripts/build_qt/configure_qt.ps1 -Destination QT_OUT/
Copy-Item -Path $SSL_PATH -Recurse -Destination QT_OUT/
zip -r qt6_win.zip QT_OUT
