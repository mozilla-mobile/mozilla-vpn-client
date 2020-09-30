#!/bin/bash

. $(dirname $0)/commons.sh

print N "This script compiles MozillaVPN for Linux"
print N ""

if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
  print G "Usage:"
  print N "\t$0"
  print N ""
  print G "Config variables:"
  print N "\tQTBIN=</path/of/the/qt/bin/folder>"
  print N ""
  exit 0
fi

if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi

if [ "$QTBIN" ]; then
  QMAKE=$QTBIN/qmake
else
  QMAKE=qmake
fi

$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

printn Y "Cleaning the folder... "
make distclean &>/dev/null;
print G "done."

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

printn Y "Computing the version... "
VERSION=$(cat src/src.pro | grep VERSION | grep defined | cut -d= -f2 | tr -d \ ).$(date +"%Y%m%d%H%M")
print G $VERSION

print Y "Configuring the build (qmake)..."
$QMAKE \
  VERSION=$VERSION \
  CONFIG+=static \
  QTPLUGIN+=qsvg \
  QTPLUGIN.imageformats+=png \
  PREFIX=$PWD/.tmp/usr || die "Compilation failed"

print Y "Compiling..."
make -j8
print G "Compilation completed!"

print Y "Installation..."
make install
print G "Installation completed!"

printn Y "Copying extra files... "
mkdir -p .tmp/usr/share/applications || die "Failed to create dir .tmp/usr/share/applications"
cp linux/extra/MozillaVPN.desktop .tmp/usr/share/applications || die "Failed to copy the desktop file"
mkdir -p .tmp/usr/share/doc/mozillavpn || die "Failed to create dir .tmp/usr/share/doc/mozillavpn"
cp linux/extra/copyright .tmp/usr/share/doc/mozillavpn || die "Failed to copy the copyright file"
print G "done."

printn Y "Calculating the package size... "
SIZE=$(du -s .tmp | cut -d\t -f1)
print G $SIZE

printn Y "Generating MD5 hashes... "
mkdir -p .tmp/DEBIAN || die "Failed to create the DEBIAN directory"
find .tmp -type f | grep -v DEBIAN | while read FILE; do echo $(md5sum $FILE | cut -d\  -f1) $(echo $FILE | sed 's/\.tmp\///'); done > .tmp/DEBIAN/md5sums
print G "done."

printn Y "Generating the control file... "
cat linux/extra/control | sed "s/VERSION/$VERSION/" | sed "s/SIZE/$SIZE/" > .tmp/DEBIAN/control
print G "done."

print Y "Generating the debian/ubuntu package..."
fakeroot dpkg-deb -b .tmp mozillavpn-$VERSION.deb || die "Failed to run fakeroot + dpkg-deb"

print G "All done!"
