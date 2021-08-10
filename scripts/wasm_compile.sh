#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

if [ -f .env ]; then
  . .env
fi

WASM_QT_PATH=

helpFunction() {
  print G "Usage:"
  print N "\t$0 <qt/wasm/path>"
  print N ""
  exit 0
}

print N "This script compiles MozillaVPN for WebAssembly"
print N ""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
  *)
    if [[ "$WASM_QT_PATH" ]]; then
      helpFunction
    fi

    WASM_QT_PATH=$1
    shift
    ;;
  esac
done

if ! [ -d "$WASM_QT_PATH" ]; then
  helpFunction
fi

if ! [ -d "src" ] || ! [ -d "wasm" ]; then
  die "This script must be executed at the root of the repository."
fi

printn Y "Extract the project version... "
SHORTVERSION=$(cat version.pri | grep VERSION | grep defined | cut -d= -f2 | tr -d \ )
FULLVERSION=$(echo $SHORTVERSION | cut -d. -f1).$(date +"%Y%m%d%H%M")
print G "$SHORTVERSION - $FULLVERSION"

QMAKE=$WASM_QT_PATH/qmake
[ -f "$QMAKE" ] || die "Unable to find qmake at the path $QMAKE"

printn Y "Setting PATH var... "
export PATH=$PATH:$WASM_QT_PATH
print G "done."

printn Y "Checking emscripten... "
em++ --version &>/dev/null || die "em++ not found. Have you forgotten to load emsdk_env.sh?"
em++ --version 2>&1 | grep 1.39.8 &>/dev/null || die "em++ doesn't match the required version: 1.39.8"
print G "done."

$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

print Y "Importing translation files..."
git submodule update --remote --depth 1 i18n || die "Failed to fetch newest translation files"
python3 scripts/importLanguages.py || die "Failed to import languages"

print Y "Generating glean samples..."
python3 scripts/generate_glean.py || die "Failed to generate glean samples"

print Y "Configuring the project via qmake..."
$QMAKE CONFIG-=debug  CONFIG-=debug_and_release CONFIG+=release BUILD_ID=$FULLVERSION || die "Compilation failed"

print Y "Compiling..."
make -j8 || die "Compilation failed"

printn Y "Moving files... "
mv src/mozillavpn.wasm wasm || die "Failed"
mv src/mozillavpn.js wasm || die "Failed"
mv src/qtloader.js wasm || die "Failed"
print G "done."
