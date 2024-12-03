#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

print N "This script generates 'ts' files for the Mozilla VPN app."
print N ""

# When the `-a`/`--all` flag is present, .ts files are created
# for every addon.
# Without an argument of `-a` or `--all`, .ts files will NOT
# be created for addons that use the shared strings file.
KEEP_ALL_TS_FILES=0
# Parse Script arguments
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
        (-a | --all)
        KEEP_ALL_TS_FILES=1
        shift
        ;;
    esac
done

cd $(dirname $0)/../.. || die

printn Y "Branch name: "
BRANCHNAME="$(git rev-parse --abbrev-ref HEAD 2>/dev/null)"
print G "$BRANCHNAME"

printn Y "Caching the dep scripts... "
mkdir -p cache || die
cp scripts/utils/generate_strings.py cache || die
print G "done."

mkdir -p translations/generated || die

printn Y "Generating strings... "
python3 cache/generate_strings.py src/translations/strings.yaml -o translations/generated
print G "done."

# Helper method - scan a directory and generate a QRC file which includes all QML files.
generate_qml2qrc() {
  cat << EOF
<RCC>
  <qresource prefix="/dummy">
$(find $1 -name '*.qml' -printf '    <file>../../%p</file>\n')
  </qresource>
</RCC>
EOF
}

printn Y "Generating a dummy PRO file... "
cat > translations/generated/dummy_ts.pro << EOF
HEADERS += i18nstrings.h
HEADERS += \$\$files(../../src/*.h, true)
HEADERS += \$\$files(../../nebula/*.h, true)

SOURCES += i18nstrings_p.cpp
SOURCES += ../i18nstrings.cpp
SOURCES += \$\$files(../../src/*.cpp, true)
SOURCES += \$\$files(../../nebula/*.cpp, true)

TRANSLATIONS += translations.ts

RESOURCES += \$\$files(src.qrc, true)
RESOURCES += \$\$files(nebula.qrc, true)
EOF
print G "done"

printn Y "Scanning for QML content..."
generate_qml2qrc src/ui > translations/generated/src.qrc
generate_qml2qrc nebula > translations/generated/nebula.qrc

QT_HOST_BINS=$(qmake6 -query QT_HOST_BINS)

print Y "Generating the main translation file... "
${QT_HOST_BINS}/lupdate translations/generated/dummy_ts.pro -ts translations.ts || die

print Y "Generating strings for addons... "
cmake cmake -S $(pwd)/addons -B build-addons/
cmake --build build-addons/
mkdir -p addon_ts || die
cp build-addons/*.ts addon_ts

for branch in $(git branch -r | grep origin/releases); do
  echo "Checking out to branch $branch"
  git checkout $branch || die

  printn Y "Generating strings... "
  if [ -f translations/strings.yaml ]; then
    python3 cache/generate_strings.py -o translations/generated translations/strings.yaml || die
  elif [ -f src/translations/strings.yaml ]; then
    python3 cache/generate_strings.py -o translations/generated src/translations/strings.yaml || die
  else
    die "Unable to find the strings.yaml"
  fi

  printn Y "Scanning for new strings..."
  generate_qml2qrc src/ui > translations/generated/src.qrc
  generate_qml2qrc nebula > translations/generated/nebula.qrc
  ${QT_HOST_BINS}/lupdate translations/generated/dummy_ts.pro -ts branch.ts || die
  ${QT_HOST_BINS}/lconvert -i translations.ts branch.ts -o tmp.ts || die
  mv tmp.ts translations.ts || die
  rm branch.ts || die

  print Y "Importing addon strings from $branch..."
  mkdir -p build-addons-$branch/
  cmake -S addons/ -B build-addons-$branch/
  cmake --build build-addons-$branch/
  ts_files="build-addons-$branch/*.ts"

  for f in $ts_files
  do
    ts_name=$(basename "$f")
    if [ -f "addon_ts/${ts_name}" ]; then
      print Y "File ${ts_name} exists, updating with branch strings..."
      ${QT_HOST_BINS}/lconvert -i "addon_ts/${ts_name}" "$f" -o tmp.ts || die
      mv tmp.ts "addon_ts/${ts_name}"
    else
      print Y "File ${ts_name} does not exist, copying over..."
      cp "$f" addon_ts/
    fi
    rm $f || die
  done
done

# When creating translation files for l10n repo, remove any addon-specific files that use
# shared strings, as they are translated via `strings.yaml`, which is set up for translation in
# `build.py` (which like the addon ts files, is created above when addon's cmake calls `build.py`)"
if [ $KEEP_ALL_TS_FILES == 0 ]; then
  print Y "Checking for .ts files using shared strings"
  for ts_file in ./addon_ts/*
  do
    if grep vpn.commonString $ts_file > /dev/null; then
      print G "Deleting file because found shared strings: $ts_file"
      rm $ts_file
    fi
  done
fi

if [ "$BRANCHNAME" ]; then
  printn Y "Go back to the initial branch... "
  git checkout "$BRANCHNAME" &>/dev/null || die
  print G "done."
fi

printn Y "Remove cache... "
rm -rf cache || die
print G "done."

exit 0
