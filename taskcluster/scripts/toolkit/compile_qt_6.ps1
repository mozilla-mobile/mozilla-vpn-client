

# Enter the DEV Shell
./$PSScriptRoot/enter_dev_shell.ps1

$BUILD_TOOLS_DIR = "C:\MozillaVPNBuild"
$QT_SRC_SUBDIR = "qt-everywhere-src-6.2.3"

$env:Path += ";$env:PERL_PATH"


# Debugging stuff for basti 
write-output("-- ENV --")
gci env:* | sort-object name
write-output("-- ENV --")
write-output("-- DIR --")
Get-ChildItem .
Get-ChildItem fetches
write-output("-- DIR --")

# Enter QT source directory
Set-Location $env:QT_PATH

./configure `
  -static  `
  -opensource  `
  -release  `
  -no-dbus   `
  -no-feature-qdbus  `
  -confirm-license  `
  -strip  `
  -silent  `
  -nomake tests  `
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
# TODO: openssl dep so we can link it. 
#  -openssl-linked `
#  -I $BUILD_TOOLS_DIR\include `
#  -L $BUILD_TOOLS_DIR\lib `
 # -prefix QTBIN `

