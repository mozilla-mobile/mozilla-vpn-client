#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

. $(dirname $0)/commons.sh

print N "This script generates 'ts' files for apps under src/apps."
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

for project in src/apps/*; do
  project=$(basename $project)

  [ $project = "auth_tests" ] || [ $project = "template" ] || [ "$project" = "unit_tests" ] || [ "$project" = ".DS_Store" ] && continue

  mkdir -p translations/generated/$project || die

  printn Y "$project - Generating strings... "
  python3 cache/generate_strings.py src/apps/$project/translations/strings.yaml src/shared/translations/strings.yaml -o translations/generated/$project -p $project
  print G "done."

  printn Y "$project - Generating a dummy PRO file... "
  cat > translations/generated/$project/dummy_ts.pro << EOF
HEADERS += i18nstrings.h
HEADERS += \$\$files(../../../src/shared/*.h, true)
HEADERS += \$\$files(../../../src/apps/$project/*.h, true)
HEADERS += \$\$files(../../../nebula/*.h, true)

SOURCES += i18nstrings_p.cpp
SOURCES += ../../i18nstrings.cpp
SOURCES += \$\$files(../../../src/shared/*.cpp, true)
SOURCES += \$\$files(../../../src/apps/$project/*.cpp, true)
SOURCES += \$\$files(../../../nebula/*.cpp, true)

TRANSLATIONS += translations.ts

RESOURCES += \$\$files(../../../src/shared/*.qrc, true)
RESOURCES += \$\$files(../../../src/apps/$project/*.qrc, true)
RESOURCES += \$\$files(../../../nebula/*.qrc, true)
EOF
  print G "done"

  QT_HOST_BINS=$(qmake6 -query QT_HOST_BINS)

  print Y "$project - Generating the main translation file... "
  ${QT_HOST_BINS}/lupdate translations/generated/$project/dummy_ts.pro -ts translations.ts || die

  printn Y "$project - Generating strings for addons... "
  cmake cmake -S $(pwd)/addons -B build-addons/
  cmake --build build-addons/
  mkdir -p addon_ts || die
  cp build-addons/*.ts addon_ts
  print G "done."

  for branch in $(git branch -r | grep origin/releases); do
    git checkout $branch &>/dev/null || die

    printn Y "Importing main strings from $branch..."
    if [ -f translations/strings.yaml ]; then
      if [ $project = "vpn" ]; then
        python3 cache/generate_strings.py -o translations/generated/$project -p $project translations/strings.yaml || die
      else
        echo "# dummy" > translations/generated/dummy_strings.yaml
        python3 cache/generate_strings.py -o translations/generated/$project -p $project translations/generated/dummy_strings.yaml || die
      fi
    elif [ -f src/apps/$project/translations/strings.yaml ]; then
      EXTRA_STRINGS=
      if [ -f src/shared/translations/strings.yaml ]; then
        EXTRA_STRINGS=src/shared/translations/strings.yaml
      fi
      python3 cache/generate_strings.py -o translations/generated/$project -p $project src/apps/$project/translations/strings.yaml $EXTRA_STRINGS || die
    else
      die "Unable to find the strings.yaml"
    fi

    ${QT_HOST_BINS}/lupdate translations/generated/$project/dummy_ts.pro -ts branch.ts || die
    ${QT_HOST_BINS}/lconvert -i translations.ts branch.ts -o tmp.ts || die
    mv tmp.ts translations.ts || die
    rm branch.ts || die

    if [ "$project" = "vpn" ]; then
      printn Y "Importing addon strings from $branch..."
      if [ -f "scripts/addon/generate_all.py" ]; then
        # Use the old python scripts to generate addons.
        python3 scripts/addon/generate_all.py
        ts_files="addons/generated/addons/*.ts"
      elif [ -f "addons/CMakeLists.txt" ]; then
        # Use the CMake project to generate addons.
        mkdir -p build-addons-$branch/
        cmake -S addons/ -B build-addons-$branch/
        cmake --build build-addons-$branch/
        ts_files="build-addons-$branch/*.ts"
      else
        # No addons to process.
        ts_files=
      fi

      for f in $ts_files
      do
        ts_name=$(basename "$f")
        if [ -f "addon_ts/${ts_name}" ]; then
          printn Y "File ${ts_name} exists, updating with branch strings..."
          ${QT_HOST_BINS}/lconvert -i "addon_ts/${ts_name}" "$f" -o tmp.ts || die
          mv tmp.ts "addon_ts/${ts_name}"
        else
          printn Y "File ${ts_name} does not exist, copying over..."
          cp "$f" addon_ts/
        fi
        rm $f || die
      done
    fi
  done

  if [ "$BRANCHNAME" ]; then
    printn Y "Go back to the initial branch... "
    git checkout "$BRANCHNAME" &>/dev/null || die
    print G "done."
  fi

  mv translations.ts "translations_$project.ts"
done

printn Y "Remove cache... "
rm -rf cache || die
print G "done."

for project in src/apps/*; do
  project=$(basename $project)
  [ $project = "auth_tests" ] || [ $project = "template" ] || [ "$project" = "unit_tests" ] && continue

  printn Y "Generated file: "
  print G translations_$project.ts
done

exit 0
