#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

BRANCH=
PPA=mozbaku/mozillavpn
VERSION=1
RELEASE=focal

if [ -f .env ]; then
  . .env
fi

helpFunction() {
  print G "Usage:"
  print N "\t$0 [-b|--branch <branch>] [-p|--ppa <ppa>] [-k|--key <sign_key_id>] [-r|--release <release>] [-v|--version <id>]"
  print N ""
  print N "By default, the ppa is: mozbaku/mozillavpn"
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
  -b | --branch)
    BRANCH="--branch $2"
    shift
    shift
    ;;
  -p | --ppa)
    PPA="$2"
    shift
    shift
    ;;
  -k | --key)
    KEY="--sign-key=$2"
    shift
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

printn Y "Computing the version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION"

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print N "Checking out the code from the git repository..."
git clone --depth 1 https://github.com/bakulf/mozilla-vpn-client .tmp/mozillavpn-$SHORTVERSION $BRANCH || die "Failed"

printn Y "Changing directory..."
cd .tmp/mozillavpn-$SHORTVERSION || die "Failed"
print G "done."

print Y "Update the submodules..."
git submodule init || die "Failed"
git submodule update --remote --depth 1 i18n || die "Failed"
git submodule update --remote --depth 1 3rdparty/wireguard-tools || die "Failed"
print G "done."

printn Y "Moving the debian template folder in the root of the repo..."
mv linux/debian .. || die "Failed"
print G "done."

printn Y "Archiving the source code..."
tar cvfz ../mozillavpn_$SHORTVERSION.orig.tar.gz . || die "Failed"

print Y "Configuring the debian package for $RELEASE..."
rm -rf debian || die "Failed"
cp -r ../debian . || die "Failed"
mv debian/changelog.template debian/changelog || die "Failed"
sed -i -e "s/VERSION/$VERSION/g" debian/changelog || die "Failed"
sed -i -e "s/RELEASE/$RELEASE/g" debian/changelog || die "Failed"
sed -i -e "s/DATE/$(date -R)/g" debian/changelog || die "Failed"
debuild -S || die "Failed"

print Y "Upload the changes to the ppa..."
cd .. || die "Failed"
dput ppa:$PPA mozillavpn*.changes || die "Failed"

print G "All done!"
