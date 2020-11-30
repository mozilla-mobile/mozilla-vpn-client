#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

JOBS=8

if [ -f .env ]; then
  . .env
fi

helpFunction() {
  print G "Usage:"
  print N "\t$0 [-j|--jobs <jobs>]"
  print N ""
  exit 0
}

print N "This script compiles MozillaVPN for Linux"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -j | --jobs)
    JOBS="$2"
    shift
    shift
    ;;
  *)
    helpFunction
    ;;
  esac
done

if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi

qmake -v &>/dev/null || die "qmake doesn't exist or it fails"

printn Y "Cleaning the folder... "
make distclean &>/dev/null;
print G "done."

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print Y "Importing translation files..."
python3 scripts/importLanguages.py || die "Failed to import"

printn Y "Computing the version... "
VERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ ).$(date +"%Y%m%d%H%M")
print G $VERSION

print Y "Configuring the build (qmake)..."
qmake\
  VERSION=$VERSION \
  CONFIG+=static \
  QTPLUGIN+=qsvg \
  QTPLUGIN.imageformats+=png \
  PREFIX=$PWD/.tmp/usr || die "Compilation failed"

print Y "Compiling..."
make -j $JOBS
print G "Compilation completed!"

print Y "Installation..."
make install
print G "Installation completed!"

printn Y "Copying extra files... "
mkdir -p .tmp/usr/share/applications || die "Failed to create dir .tmp/usr/share/applications"
cp linux/extra/MozillaVPN.desktop .tmp/usr/share/applications || die "Failed to copy the desktop file"
mkdir -p .tmp/etc/xdg/autostart || die "Failed to create dir .tmp/etc/xdg/autostart"
cp linux/extra/MozillaVPN-startup.desktop .tmp/etc/xdg/autostart || die "Failed to copy the desktop file"
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
