#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

REVISION=1
RELEASE=
GITREF=
QTVERSION="qt6"
SOURCEONLY=N
PPA_URL=
DPKG_SIGN="--no-sign"
RPM=N
DEB=N

if [ -f .env ]; then
  . .env
fi

helpFunction() {
  print G "Usage: $0 [OPTIONS]"
  print N ""
  print N "Build options:"
  print N "  -r, --release DIST     Build packages for distribution DIST"
  print N "  -g, --gitref REF       Generated version suffix from REF"
  print N "  -v, --version REV      Set package revision to REV"
  print N "      --beineri          Build using Stephan Binner's Qt5.15 PPA"
  print N "      --qt5              Build using Qt5 packages"
  print N "      --qt6              Build using Qt6 packages (default)"
  print N "      --source           Build source packages only (no binary)"
  print N "      --ppa URL          Upload source packages to PPA at URL (implies: --source)"
  print N ""
  print N "Signing options:"
  print N "      --sign             Enable package signing (default: disabled)"
  print N "  -k, --sign-key KEYID   Enable package using using GPG key of KEYID"
  print N "      --no-sign          Disable package signing" 
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
  -r | --release)
    RELEASE+=" $2"
    shift
    shift
    ;;
  -g | --gitref)
    GITREF="$2"
    shift
    shift
    ;;
  -v | --version)
    REVISION="$2"
    shift
    shift
    ;;
  --beineri)
    QTVERSION="beineri"
    shift
    ;;
  --qt5)
    QTVERSION="qt5"
    shift
    ;;
  --qt6)
    QTVERSION="qt6"
    shift
    ;;
  --source)
    RELEASE="bionic focal jammy fedora"
    SOURCEONLY=Y
    shift
    ;;
  --ppa)
    SOURCEONLY=Y
    PPA_URL="$2"
    shift
    shift
    ;;
  --sign)
    DPKG_SIGN=""
    shift
    ;;
  -k | --sign-key)
    DPKG_SIGN="--sign-key=$2"
    shift
    shift
    ;;
  --no-sign)
    DPKG_SIGN="--no-sign"
    shift
    ;;
  *)
    helpFunction
    ;;
  esac
done

# Fall back to the host operating system if no release was specified
if [ -z "$RELEASE" ]; then
  . /etc/os-release
  RELEASE="$VERSION_CODENAME"
fi

printn Y "Computing the version... "
# To explain this ugly pile of regex:
#  1. The grep statement matches a cmake project(...) directive.
#  2. awk breaks it into tokens separated by whitespace.
#  3. print whatever token we find after "VERSION"
SHORTVERSION=$(grep -zoE 'project\s*\([^\(\)]*\)' CMakeLists.txt |
               awk '{ for (x=1;x<NF;x++) if ($x=="VERSION") print $(x+1) }')

# Adjust the package version if a gitref was provided:
#  - Pull requests are suffixed with "~pr<Pull Request Number>"
#  - Release tags force the version to match the tag.
#  - Release branches are suffixed with "~rc<# of commits since main>"
#  - The main branch sets a nightly date code.
if [[ "$GITREF" =~ ^refs/pull/([0-9]+)/merge ]]; then
  SHORTVERSION="${SHORTVERSION}~pr${BASH_REMATCH[1]}"
elif [[ "$GITREF" =~ ^refs/tags/v([0-9a-z.]+) ]]; then
  SHORTVERSION=${BASH_REMATCH[1]}
elif [[ "$GITREF" =~ ^refs/heads/releases/([0-9][^/]*) ]]; then
  git fetch --unshallow
  RCVERSION="~rc$(git rev-list --count --first-parent origin/main..HEAD)"
  SHORTVERSION="${BASH_REMATCH[1]}${RCVERSION}"
elif [[ "$GITREF" == "refs/heads/main" ]]; then
  SHORTVERSION="${SHORTVERSION}~nightly$(date +%Y%m%d)"
fi
WORKDIR=mozillavpn-${SHORTVERSION}
print G "${SHORTVERSION}"

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print Y "Update the submodules..."
git submodule init || die "Failed"
git submodule update --remote --depth 1 i18n || die "Failed"
git submodule update --remote --depth 1 3rdparty/wireguard-tools || die "Failed"
print G "done."

print G "Creating the orig tarball"

printn N "Creating the working directory... "
[ -e "CMakeCache.txt" ] && die "Source directory is dirty, refusing to build packages"
RSYNC_EXCLUDE_DIRS=
for CACHEFILE in $(find . -maxdepth 2 -name 'CMakeCache.txt' -printf '%P\n'); do
  RSYNC_EXCLUDE_DIRS="${RSYNC_EXCLUDE_DIRS} --exclude=$(dirname ${CACHEFILE})"
done
mkdir -p .tmp/${WORKDIR} || die "Failed"
rsync -a --exclude='.*' ${RSYNC_EXCLUDE_DIRS} . .tmp/${WORKDIR} || die "Failed"
print G "done."
cd .tmp

print Y "Generating glean samples..."
(cd $WORKDIR && python3 scripts/utils/generate_glean.py) || die "Failed to generate glean samples"

printn Y "Downloading Go dependencies..."
(cd $WORKDIR/linux/netfilter && go mod vendor)
print G "done."

printn Y "Downloading Rust dependencies (extension)..."
(cd $WORKDIR/extension/bridge && mkdir -p .cargo && cargo vendor > .cargo/config.toml)
print G "done."

printn Y "Downloading Rust dependencies (signature)..."
(cd $WORKDIR/signature && mkdir -p .cargo && cargo vendor > .cargo/config.toml)
print G "done."

printn Y "Removing the packaging templates... "
rm -f $WORKDIR/linux/mozillavpn.spec || die "Failed"
rm -rf $WORKDIR/linux/debian || die "Failed"
print G "done."

printn Y "Archiving the source code... "
TAR_OPTIONS="--mtime=$(git log -1 --format=%cI) --owner=root:0 --group=root:0 --sort=name"
LC_ALL=C tar cfz mozillavpn_$SHORTVERSION.orig.tar.gz $TAR_OPTIONS $WORKDIR || die "Failed"
print G "done."

## Generate the spec file for building RPMs
build_rpm_spec() {
cat << EOF > mozillavpn.spec
%define _srcdir .
Version: $SHORTVERSION
Release: $REVISION
Source0: mozillavpn_$SHORTVERSION.orig.tar.gz
$(sed -e '/^%prep/ a %autosetup' ../linux/mozillavpn.spec | grep -v -e "^Version:" -e "^Release" -e "^%define")
EOF
}

## For a given distro, build the DSC and debian tarball.
build_deb_source() {
  local distro=$1
  local buildrev=${distro}${REVISION}

  print Y "Building sources for $distro ($buildtype)..."
  rm -rf $WORKDIR/debian || die "Failed"
  cp -r ../linux/debian $WORKDIR || die "Failed"

  mv $WORKDIR/debian/rules.$QTVERSION $WORKDIR/debian/rules
  mv $WORKDIR/debian/control.$QTVERSION $WORKDIR/debian/control
  rm $WORKDIR/debian/control.*
  rm $WORKDIR/debian/rules.*

  mv $WORKDIR/debian/changelog.template $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/SHORTVERSION/$SHORTVERSION/g" $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/VERSION/$buildrev/g" $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/RELEASE/$distro/g" $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/DATE/$(date -R)/g" $WORKDIR/debian/changelog || die "Failed"

  (cd $WORKDIR && dpkg-buildpackage --build=source $DPKG_SIGN --no-check-builddeps) || die "Failed"
}

## Prepare the distribution's packaging sources
for distro in $RELEASE; do
  case "$distro" in
    fedora|rpm)
      print Y "Building RPM packages for $distro"
      build_rpm_spec
      rpmbuild --define "_srcrpmdir $(pwd)" --define "_sourcedir $(pwd)" -bs mozillavpn.spec
      ;;

    *)
      print Y "Building Debian packages for $distro"
      build_deb_source $distro
      ;;
  esac
done

print Y "Cleaning up working directory..."
rm -rf $WORKDIR || die "Failed"

## Handle PPA Uploads
if [ ! -z "$PPA_URL" ]; then
  print Y "Uploading sources to $PPA_URL"
  for changeset in $(find . -type f -name '*_source.changes'); do
    dput "$PPA_URL" $changeset
  done
fi

## Build Binary packages
if [ "$SOURCEONLY" != "Y" ]; then
  for changeset in $(find . -type f -name '*_source.changes'); do
    print Y "Building binary package from $changeset"
    dpkg-source -x ${changeset%_source.changes}.dsc
    (cd $WORKDIR && dpkg-buildpackage --build=binary $DPKG_SIGN) || die "Failed"
    rm -rf $WORKDIR
  done
fi

print G "All done."
