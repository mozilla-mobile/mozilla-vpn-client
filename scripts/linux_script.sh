#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

VERSION=1
RELEASE=focal
BUILDTYPE=prod
SOURCEONLY=N
RPM=N
DEB=N

if [ -f .env ]; then
  . .env
fi

helpFunction() {
  print G "Usage:"
  print N "\t$0 [-r|--release <release>] [-v|--version <id>] [-s|--stage] [--source]"
  print N ""
  print N "By default, the release is 'focal'"
  print N "The default version is 1, but you can recreate packages using the same code version changing the version id."
  print N ""
  exit 0
}

print N "This script compiles MozillaVPN and creates a debian/ubuntu package"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -s | --stage)
    BUILDTYPE=stage
    shift
    ;;
  -r | --release)
    RELEASE="$2"
    shift
    shift
    ;;
  -v | --version)
    VERSION="$2"
    shift
    shift
    ;;
  --source)
    SOURCEONLY=Y
    shift
    ;;
  *)
    helpFunction
    ;;
  esac
done

printn Y "Computing the version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
WORKDIR=mozillavpn-$SHORTVERSION
print G "$SHORTVERSION - $FULLVERSION"

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print Y "Update the submodules..."
git submodule init || die "Failed"
git submodule update --remote --depth 1 i18n || die "Failed"
git submodule update --remote --depth 1 3rdparty/wireguard-tools || die "Failed"
print G "done."

print G "Creating the orig tarball"

printn N "Creating the working directory... "
mkdir -p .tmp/$WORKDIR || die "Failed"
cp -R * .tmp/$WORKDIR 2>/dev/null || die "Failed"
cd .tmp
print G "done."

print Y "Importing translation files..."
LANG_ARGS=$([ "$BUILDTYPE" == "stage" ] || echo "-p")
(cd $WORKDIR && python3 scripts/importLanguages.py $LANG_ARGS) || die "Failed to import languages"

print Y "Generating glean samples..."
(cd $WORKDIR && python3 scripts/generate_glean.py) || die "Failed to generate glean samples"

print Y "Downloading Go dependencies..."
(cd $WORKDIR/linux/netfilter && go mod vendor)
print G "done."

printn Y "Removing the packaging templates... "
rm -f $WORKDIR/linux/mozillavpn.spec || die "Failed"
rm -rf $WORKDIR/linux/debian || die "Failed"
print G "done."

printn Y "Archiving the source code... "
tar cfz mozillavpn_$SHORTVERSION.orig.tar.gz $WORKDIR || die "Failed"
print G "done."

## Generate the spec file for building RPMs
build_rpm_spec() {
cat << EOF > mozillavpn.spec
Version: $SHORTVERSION
Release: $VERSION
Source0: mozillavpn_$SHORTVERSION.orig.tar.gz
$(grep -v -e "^Version:" -e "^Release" -e "^%define" ../linux/mozillavpn.spec)
EOF
}

## For a given control file, build the DSC and debian tarball.
build_deb_source() {
  local release=$1
  local buildtype=$2

  print Y "Building sources for $release ($buildtype)..."
  rm -rf $WORKDIR/debian || die "Failed"
  cp -r ../linux/debian $WORKDIR || die "Failed"

  mv $WORKDIR/debian/rules.$buildtype.$release $WORKDIR/debian/rules
  mv $WORKDIR/debian/control.$buildtype.$release $WORKDIR/debian/control
  rm $WORKDIR/debian/control.*
  rm $WORKDIR/debian/rules.*

  mv $WORKDIR/debian/changelog.template $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/SHORTVERSION/$SHORTVERSION/g" $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/VERSION/$VERSION/g" $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/RELEASE/$release/g" $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/DATE/$(date -R)/g" $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/FULLVERSION/$FULLVERSION/g" $WORKDIR/debian/rules || die "Failed"

  dpkg-source --build $WORKDIR || die "Failed"
}

## For source-only, build all the source bundles we can.
if [ "$SOURCEONLY" == "Y" ]; then
  print Y "Configuring the DEB sources..."
  for control in ../linux/debian/control.*; do
    filename=$(basename $control)
    buildtype=$(echo $filename | cut -d'.' -f2)
    release=$(echo $filename | cut -d'.' -f3)

    build_deb_source $release $buildtype

    mkdir $release-$buildtype/
    mv mozillavpn_$SHORTVERSION-$VERSION.debian.tar.* $release-$buildtype/ || die "Failed"
    mv mozillavpn_$SHORTVERSION-$VERSION.dsc $release-$buildtype/ || die "Failed"
  done

  if [ ! -z "$(which rpmbuild)" ]; then
    print Y "Configuring the RPM spec..."
    build_rpm_spec
  fi
## Otherwise, build the desired release.
else
  case "$RELEASE" in
    bionic|focal|groovy|hirsute)
      build_deb_source $RELEASE $BUILDTYPE

      print Y "Building Debian packages for $RELEASE ($BUILDTYPE)"
      (cd $WORKDIR && dpkg-buildpackage --build=binary --no-sign) || die "Failed"
      ;;
    
    fedora)
      build_rpm_spec

      print Y "Building RPM packages for $RELEASE ($BUILDTYPE)"
      rpmbuild --define "_topdir $(pwd)" --define "_sourcedir $(pwd)" -bs mozillavpn.spec
      RPM=Y
      ;;

    *)
      die "We support RELEASE focal, groovy, bionic and hirsute only"
      ;; 
  esac
fi

print Y "Cleaning up working directory..."
rm -rf $WORKDIR || die "Failed"

print G "All done."
