#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

SOURCE=
DEST=

PRODUCT="Mozilla VPN"

helpFunction() {
  print G "Usage:"
  print N "\t$0 [-s|--source <app>] [-d|--destination <pkg>]"
  print N ""
  exit 0
}
print N "This script creates a apple pkg file"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -s | --source)
    SOURCE=$2
    shift
    shift
    ;;
  -d | --destination)
    DEST=$2
    shift
    shift
    ;;
  *)
    helpFunction
    ;;
  esac
done

if [[ "$SOURCE" == "" ]] || [[ "$DEST" == "" ]];then
  helpFunction
fi

printn Y "Extract the project version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION"

printn Y "Cleaning $DEST directory... "
rm -rf "${DEST}" || die "Failed to remove the folder"
mkdir "${DEST}" || die "Failed to create the folder"
print G "done."

printn Y "Copy template files... "
cp -r macos/pkg "${DEST}/darwin" || die "Failed to copy"
chmod -R 755 "${DEST}/darwin/scripts" || die "Failed to set permissions 1"
chmod -R 755 "${DEST}/darwin/Resources" || die "Failed to set permissions 2"
chmod 755 "${DEST}/darwin/Distribution" || die "Failed to set permissions 3"
print G "done."

printn Y "Configuring the postinstall script... "
chmod -R 755 "${DEST}/darwin/scripts/postinstall"
print G "done."

printn Y "Configuring the Distribution and resources files... "
chmod -R 755 "${DEST}/darwin/Distribution"
chmod -R 755 "${DEST}/darwin/Resources/"
print G "done."

printn Y "Creating the pkg structure... "
mkdir -p ${DEST}/darwinpkg || die "Failed to configure the folder"

mkdir -p "${DEST}/darwinpkg/Applications"
cp -r "${SOURCE}" "${DEST}/darwinpkg/Applications"
[[ -d "${DEST}/darwinpkg/Applications/Mozilla VPN.app" ]] || die "The folder name must be 'Mozilla VPN.app'"
chmod -R 755 "${DEST}/darwinpkg/Applications/Mozilla VPN.app"

mkdir -p "${DEST}/package"
chmod -R 755 "${DEST}/package"

rm -rf "${DEST}/pkg"
mkdir -p "${DEST}/pkg"
chmod -R 755 "${DEST}/pkg"

print G "done."

print Y "Application installer package building started. Step 1"
pkgbuild \
  --identifier "$(cat xcode.xconfig | grep APP_ID_MACOS | cut -d= -f2)" \
  --version ${SHORTVERSION} \
  --scripts "${DEST}/darwin/scripts" \
  --root "${DEST}/darwinpkg" \
  "${DEST}/package/${PRODUCT}.pkg" || die "Failed"

print Y "Application installer package building started. Step 2"
productbuild \
  --distribution "${DEST}/darwin/Distribution" \
  --resources "${DEST}/darwin/Resources" \
  --package-path "${DEST}/package" \
  "${DEST}/pkg/MozillaVPN-${SHORTVERSION}.pkg"

while true; do
   read -p "Do you wish to sign the installer (You should have Apple Developer Certificate) [y/N]?" answer
   [[ $answer == "y" || $answer == "Y" ]] && FLAG=true && break
   [[ $answer == "n" || $answer == "N" || $answer == "" ]] && print Y "Skiped signing process." && FLAG=false && break
   echo "Please answer with 'y' or 'n'"
done
if [[ $FLAG == "true" ]]; then
  print Y "Application installer signing process started. Step 3"
  mkdir -p "${DEST}/pkg-signed"
  chmod -R 755 "${DEST}/pkg-signed"

  read -p "Please enter the Apple Developer Installer Certificate ID:" APPLE_DEVELOPER_CERTIFICATE_ID
  productsign \
    --sign "Developer ID Installer: ${APPLE_DEVELOPER_CERTIFICATE_ID}" \
    "${DEST}/pkg/MozillaVPN-${SHORTVERSION}.pkg" \
    "${DEST}/pkg-signed/MozillaVPN-${SHORTVERSION}.pkg"
    pkgutil --check-signature "${DEST}/pkg-signed/MozillaVPN-${SHORTVERSION}.pkg"
fi

