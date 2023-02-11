#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

print N "This script generates a 'ts' file for the location of the Mozilla VPN client"
print N ""

cd $(dirname $0)/../.. || die

printn Y "Branch name: "
BRANCHNAME="$(git symbolic-ref HEAD 2>/dev/null)"
BRANCHNAME=${BRANCHNAME##refs/heads/}
print G "$BRANCHNAME"

printn Y "Caching the dep scripts... "
mkdir -p cache || die
cp scripts/utils/generate_strings.py cache || die
print G "done."

printn Y "Generating strings... "
python cache/generate_strings.py translations/strings.yaml -o translations/generated
print G "done."

printn Y "Generating a dummy PRO file... "
mkdir -p translations/generated || die
cat > translations/generated/dummy_ts.pro << EOF
HEADERS += l18nstrings.h
HEADERS += \$\$files(../../src/shared/*.h, true)
HEADERS += \$\$files(../../src/apps/vpn/*.h, true)
HEADERS += \$\$files(../../nebula/*.h, true)

SOURCES += l18nstrings_p.cpp
SOURCES += ../l18nstrings.cpp
SOURCES += \$\$files(../../src/shared/*.cpp, true)
SOURCES += \$\$files(../../src/apps/vpn/*.cpp, true)
SOURCES += \$\$files(../../nebula/*.cpp, true)

TRANSLATIONS += translations.ts

RESOURCES += \$\$files(../../src/shared/*.qrc, true)
RESOURCES += \$\$files(../../src/apps/vpn/*.qrc, true)
RESOURCES += \$\$files(../../nebula/*.qrc, true)
EOF
print G "done"

QT_HOST_BINS=$(qmake6 -query QT_HOST_BINS)

print Y "Generating the main translation file... "
${QT_HOST_BINS}/lupdate translations/generated/dummy_ts.pro -ts translations.ts || die

printn Y "Generating strings for addons... "
python scripts/addon/generate_all.py
mkdir -p addon_ts || die
cp addons/generated/addons/*.ts addon_ts
print G "done."

for branch in $(git branch -r | grep origin/releases); do
  git checkout $branch &>/dev/null || die

  printn Y "Importing main strings from $branch..."
  python cache/generate_strings.py -o translations/generated translations/strings.yaml || die
  ${QT_HOST_BINS}/lupdate translations/generated/dummy_ts.pro -ts branch.ts || die
  ${QT_HOST_BINS}/lconvert -i translations.ts branch.ts -o tmp.ts || die
  mv tmp.ts translations.ts || die
  rm branch.ts || die

  if [ -f "scripts/addon/generate_all.py" ]; then
    printn Y "Importing addon strings from $branch..."
    python scripts/addon/generate_all.py
    ts_files="addons/generated/addons/*.ts"
    for f in $ts_files
    do
      ts_name=$(basename "$f")
      if [ -f "addon_ts/${ts_name}" ]; then
        printn Y "File ${ts_name} exists, updating with branch strings..."
        ${QT_HOST_BINS}/lconvert -i "addon_ts/${ts_name}" "addons/generated/addons/${ts_name}" -o tmp.ts || die
        mv tmp.ts "addon_ts/${ts_name}"
      else
        printn Y "File ${ts_name} does not exist, copying over..."
        cp "addons/generated/addons/${ts_name}" addon_ts/
      fi
    done
    rm addons/generated/addons/*.ts || die
  fi
done

printn Y "Remove cache... "
rm -rf cache || die
print G "done."

if [ "$BRANCHNAME" ]; then
  printn Y "Go back to the initial branch... "
  git checkout "$BRANCHNAME" &>/dev/null || die
  print G "done."
fi

printn Y "Generated file: "
print G translations.ts
