#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

. $(dirname $0)/../utils/commons.sh

REVISION=1
RELEASE=
GITREF=
DPKG_SIGN="--no-sign"

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
  print N "      --source           Deprecated option (ignored - always true)"
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
    RELEASE="$2"
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
  --source)
    # Ignored
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
SHORTVERSION=$(cat version.txt)

# Adjust the package version if a gitref was provided:
#  - Pull requests are suffixed with "~pr<Pull Request Number>"
#  - Release tags force the version to match the tag.
#  - Release branches are suffixed with "~rc<# of commits since main>"
#  - The main branch sets a builds date code.
if [[ "$GITREF" =~ ^refs/pull/([0-9]+)/merge ]]; then
  SHORTVERSION="${SHORTVERSION}~pr${BASH_REMATCH[1]}"
elif [[ "$GITREF" =~ ^refs/tags/v([0-9a-z.]+) ]]; then
  SHORTVERSION=${BASH_REMATCH[1]}
elif [[ "$GITREF" =~ ^refs/heads/releases/([0-9][^/]*) ]]; then
  SHORTVERSION="${BASH_REMATCH[1]}~rc$(date -u +%Y%m%d%H%M%S)"
elif [[ "$GITREF" == "refs/heads/main" ]]; then
  SHORTVERSION="${SHORTVERSION}~build$(date -u +%Y%m%d)"
fi
WORKDIR=mozillavpn-${SHORTVERSION}
print G "${SHORTVERSION}"

rm -rf .tmp || die "Failed to remove the temporary directory"
mkdir .tmp || die "Failed to create the temporary directory"

print Y "Get the submodules..."
git submodule update --init --recursive || die "Failed to init submodules"

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

print Y "Generating Glean (qtglean) files..."
(cd $WORKDIR && python3 qtglean/glean_parser_ext/run_glean_parser.py) || die "Failed to generate Glean (qtglean) files"

printn Y "Downloading Go dependencies..."
(cd $WORKDIR/linux/netfilter && go mod vendor)
print G "done."

printn Y "Downloading Rust dependencies (extension)..."
(cd $WORKDIR/extension/bridge && mkdir -p .cargo && cargo vendor > .cargo/config.toml)
print G "done."

printn Y "Downloading Rust dependencies (signature)..."
(cd $WORKDIR/signature && mkdir -p .cargo && cargo vendor > .cargo/config.toml)
print G "done."

printn Y "Downloading Rust dependencies (qtglean)..."
(cd $WORKDIR/qtglean && mkdir -p .cargo && cargo vendor > .cargo/config.toml)
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
build_rpm_source() {
  cat << EOF > mozillavpn.spec
%define _srcdir .
Version: $SHORTVERSION
Release: $REVISION
Source0: mozillavpn_$SHORTVERSION.orig.tar.gz
$(sed -e '/^%prep/ a %autosetup' ../linux/mozillavpn.spec | grep -v -e "^Version:" -e "^Release" -e "^%define")
EOF

  # Build the source RPM only if the rpmbuild tool can be found.
  if which rpmbuild 2>/dev/null; then
    rpmbuild --define "_srcrpmdir $(pwd)" --define "_sourcedir $(pwd)" -bs mozillavpn.spec
  fi
}

## Update the flatpak manifest to use the source tarball.
build_flatpak_manifest() {
  # Copy the flatpak manifests
  cp -r ../linux/flatpak . || die "Failed"

  # Truncate the YAML at the sources, and replace it with the tarball archive
  sed -e '1,/[[:space:]]\+sources:/!d' ../linux/flatpak/org.mozilla.vpn.yml > flatpak/org.mozilla.vpn.yml
  cat << EOF >> flatpak/org.mozilla.vpn.yml
      - type: archive
        path: ../mozillavpn_$SHORTVERSION.orig.tar.gz
        sha256: $(sha256sum ../mozillavpn_$SHORTVERSION.orig.tar.gz | awk '{print $1}')
EOF
}

## Build the DSC and debian tarball.
build_deb_source() {
  rm -rf $WORKDIR/debian || die "Failed"
  cp -r ../linux/debian $WORKDIR || die "Failed"

  mv $WORKDIR/debian/changelog.template $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/SHORTVERSION/$SHORTVERSION/g" $WORKDIR/debian/changelog || die "Failed"
  sed -i -e "s/DATE/$(date -R)/g" $WORKDIR/debian/changelog || die "Failed"

  (cd $WORKDIR && dpkg-buildpackage --build=source $DPKG_SIGN --no-check-builddeps) || die "Failed"
}

print Y "Building RPM sources"
build_rpm_source

print Y "Building Debian sources"
build_deb_source

print Y "Building Flatpak sources"
build_flatpak_manifest

print Y "Cleaning up working directory..."
rm -rf $WORKDIR || die "Failed"

print G "All done."
exit 0
