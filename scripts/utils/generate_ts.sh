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
python cache/generate_strings.py -o translations/generated -g src/ui/guides -t src/ui/tutorials
print G "done."

printn Y "Generating a dummy PRO file... "
cat > translations/generated/dummy.pro << EOF
HEADERS += l18nstrings.h
SOURCES += l18nstrings_p.cpp
SOURCES += ../l18nstrings.cpp
TRANSLATIONS += translations.ts
HEADERS += \$\$files(../../src/*.h, true)
SOURCES += \$\$files(../../src/*.cpp, true)
RESOURCES += \$\$files(../../src/*.qrc, true)
HEADERS += \$\$files(../../nebula/*.h, true)
SOURCES += \$\$files(../../nebula/*.cpp, true)
RESOURCES += \$\$files(../../nebula/*.qrc, true)
EOF
print G "done"

print Y "Generating the main translation file... "
lupdate translations/generated/dummy.pro -ts translations.ts || die

for branch in $(git branch -r | grep origin/releases); do
  printn Y "Importing strings from $branch..."

  git checkout $branch &>/dev/null || die
  PARAMS=
  [ -d src/ui/guides ] && PARAMS="$PARAMS -g src/ui/guides"
  [ -d src/ui/tutorials ] && PARAMS="$PARAMS -t src/ui/tutorials"
  python cache/generate_strings.py -o translations/generated $PARAMS || die
  lupdate translations/generated/dummy.pro -ts branch.ts || die
  lconvert -i translations.ts branch.ts -o tmp.ts || die
  mv tmp.ts translations.ts || die
  rm branch.ts || die
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
