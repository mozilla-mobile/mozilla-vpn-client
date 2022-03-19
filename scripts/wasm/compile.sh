#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/../utils/commons.sh

if [ -f .env ]; then
  . .env
fi

DEBUG=

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  -d | --debug)
    DEBUG=1
    shift
    ;;
  -h | --help)
    helpFunction
    ;;
  esac
done

helpFunction() {
  print G "Usage:"
  print N "\t$0 [-d|--debug]"
  print N ""
  exit 0
}

print N "This script compiles MozillaVPN for WebAssembly"
print N ""

if ! [ -d "src" ] || ! [ -d "wasm" ]; then
  die "This script must be executed at the root of the repository."
fi

printn Y "Extract the project version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION"

printn Y "Checking emscripten... "
em++ --version &>/dev/null || die "em++ not found. Have you forgotten to load emsdk_env.sh?"
print G "done."

qmake -v &>/dev/null || die "qmake doesn't exist or it fails"

print Y "Importing translation files..."
git submodule update --remote --depth 1 i18n || die "Failed to fetch newest translation files"
python3 scripts/utils/import_languages.py || die "Failed to import languages"

printn Y "Mode: "
MODE=
if [ "$DEBUG" = 1 ]; then
  print G debug
  MODE="CONFIG+=debug CONFIG-=release"
else
  print G release
  MODE="CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release"
fi

print Y "Configuring the project via qmake..."
qmake $MODE BUILD_ID=$FULLVERSION || die "Compilation failed"

print Y "Compiling..."
make -j8 || die "Compilation failed"

printn Y "Moving files... "
mv src/mozillavpn.wasm wasm || die "Failed"
mv src/mozillavpn.js wasm || die "Failed"
mv src/qtloader.js wasm || die "Failed"
cp -r tools/logviewer wasm || die "Failed"
print G "done."
