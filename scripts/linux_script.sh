#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

VERSION=1
RELEASE=focal
STAGE=

if [ -f .env ]; then
  . .env
fi

helpFunction() {
  print G "Usage:"
  print N "\t$0 [-r|--release <release>] [-v|--version <id>] [-s|--stage]"
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
    STAGE=1
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
  *)
    helpFunction
    ;;
  esac
done

[ "$RELEASE" != "focal" ] && [ "$RELEASE" != "groovy" ] && [ "$RELEASE" != "bionic" ] && [ "$RELEASE" != "hirsute" ] && die "We support RELEASE focal, groovy, bionic and hirsute only"

printn Y "Computing the version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
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
mkdir -p .tmp/mozillavpn-$SHORTVERSION || die "Failed"
cp -R * .tmp/mozillavpn-$SHORTVERSION 2>/dev/null || die "Failed"
print G "done."

printn Y "Changing directory... "
cd .tmp/mozillavpn-$SHORTVERSION || die "Failed"
print G "done."

print Y "Importing translation files..."
python3 scripts/importLanguages.py $([[ "$STAGE" ]] && echo "" || echo "-p") || die "Failed to import languages"

print Y "Generating glean samples..."
python3 scripts/generate_glean.py || die "Failed to generate glean samples"

printn Y "Removing the debian template folder... "
rm -rf linux/debian || die "Failed"
print G "done."

printn Y "Archiving the source code... "
tar cfz ../mozillavpn_$SHORTVERSION.orig.tar.gz . || die "Failed"
print G "done."

print Y "Configuring the debian package for $RELEASE..."
cp -r ../../linux/debian .  || die "Failed"

if [[ "$STAGE" ]]; then
  print Y "Staging env configured"
  mv debian/rules.stage.$RELEASE debian/rules || die "Failed"
  mv debian/control.stage.$RELEASE debian/control || die "Failed"
else
  print Y "Production env configured"
  mv debian/rules.prod.$RELEASE debian/rules || die "Failed"
  mv debian/control.prod.$RELEASE debian/control || die "Failed"
fi

rm debian/control.* || die "Failed"
rm debian/rules.stage* || die "Failed"
rm debian/rules.prod* || die "Failed"

mv debian/changelog.template debian/changelog || die "Failed"
sed -i -e "s/SHORTVERSION/$SHORTVERSION/g" debian/changelog || die "Failed"
sed -i -e "s/VERSION/$VERSION/g" debian/changelog || die "Failed"
sed -i -e "s/RELEASE/$RELEASE/g" debian/changelog || die "Failed"
sed -i -e "s/DATE/$(date -R)/g" debian/changelog || die "Failed"
sed -i -e "s/FULLVERSION/$FULLVERSION/g" debian/rules || die "Failed"
debuild -uc -us || die "Failed"

print G "All done."
