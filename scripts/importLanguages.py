#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script must be executed at the root of the repository.

import argparse
import xml.etree.ElementTree as ET
import os
import sys
import shutil
import generate_strings

# Include only locales above this threshold (e.g. 70%) in production
l10n_threshold = 0.70

parser = argparse.ArgumentParser()
parser.add_argument(
    '-p', '--prod', default=False, action="store_true", dest="isprod",
    help='Build only for production locales.')
args = parser.parse_args()

def title(a, b):
    print(f"\033[96m\033[1m{a}\033[0m: \033[97m{b}\033[0m")

# Step 0
title("Step 0", "Locate the lupdate and lconvert tools...")
lupdate = shutil.which('lupdate')
if lupdate is None:
    lupdate = shutil.which('lupdate-qt5')
if lupdate is None:
    print('Unable to locate lupdate tool.')
    sys.exit(1)

lconvert = shutil.which('lconvert')
if lconvert is None:
    lconvert = shutil.which('lconvert-qt5')
if lconvert is None:
    print('Unable to locate lconvert tool.')
    sys.exit(1)

# Step 1
# Go through the i18n repo, check each XLIFF file and take
# note which locale is complete above the minimum threshold.
# Adds path of .xliff and .ts to l10n_files.
title("Step 1", "Validate the XLIFF file...")
l10n_files = []
for locale in os.listdir('i18n'):
    # Skip non folders
    if not os.path.isdir(os.path.join('i18n', locale)):
        continue

    # Skip hidden folders
    if locale.startswith('.'):
        continue

    xliff_path = os.path.join('i18n', locale, 'mozillavpn.xliff')

    # If it's the source locale (en), ignore parsing for completeness and
    # add it to the list.
    if locale == 'en':
        print(f'OK\t- en added (reference locale)')
        l10n_files.append({
            'locale': 'en',
            'ts': os.path.join('translations', f'en/mozillavpn_en.ts'),
            'xliff': xliff_path
        })
        continue

    tree = ET.parse(xliff_path)
    root = tree.getroot()

    sources = 0
    translations = 0

    for element in root.iter('{urn:oasis:names:tc:xliff:document:1.2}source'):
        sources += 1
    for element in root.iter('{urn:oasis:names:tc:xliff:document:1.2}target'):
        translations += 1

    completeness = translations/(sources*1.0)

    # Ignore locale with less than 70% of completeness for production builds
    if args.isprod and completeness < l10n_threshold:
        print(f'KO\t- {locale} is translated at {round(completeness*100, 2)}%, at least {l10n_threshold*100}% is needed')
        continue  # Not enough translations next file please

    print(f'OK\t- {locale} added ({round(completeness*100, 2)}% translated)')
    l10n_files.append({
        'locale': locale,
        'ts': os.path.join('translations', f'{locale}/mozillavpn_{locale}.ts'),
        'xliff': xliff_path
    })

# Step 2
title("Step 2", "Create folders and localization files for the languages...")
for file in l10n_files:
    dirname = os.path.dirname(file['ts'])
    if not os.path.exists(dirname):
        os.makedirs(dirname)
    locversion = os.path.join(dirname, 'locversion.plist')
    with open(locversion, 'w') as locversion_file:
        locversion_file.write(f"""<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\"
\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>LprojCompatibleVersion</key>
    <string>123</string>
    <key>LprojLocale</key>
    <string>{file['locale']}</string>
    <key>LprojRevisionLevel</key>
    <string>1</string>
    <key>LprojVersion</key>
    <string>123</string>
</dict>
</plist>""")

# Step 3
title("Step 3", "Write PRI file to import the locales that are ready...")
with open('translations/translations.pri', 'w') as pri_file:
    output = []
    output.append('TRANSLATIONS += \\ ')
    for file in l10n_files:
        output.append(f"../{file['ts']} \\ ")
    output.append('\n\n##End')

    for file in l10n_files:
        output.append(f"LANGUAGES_FILES_{file['locale']}.files += ../translations/{file['locale']}/locversion.plist")
        output.append(f"LANGUAGES_FILES_{file['locale']}.path = Contents/Resources/{file['locale']}.lproj")
        output.append(f"QMAKE_BUNDLE_DATA += LANGUAGES_FILES_{file['locale']}")
    pri_file.write('\n'.join(output))

# Step 4
title("Step 4", "Generate the Js/C++ string definitions...")
generate_strings.generateStrings()

# Step 5
title("Step 5", "Generate new ts files...")
os.system(f"{lupdate} src/src.pro")

# Step 6
title("Step 5", "Now merge translations into the files...")
for l10n_file in l10n_files:
    os.system(f"{lconvert} -i {l10n_file['ts']} -if xlf -i {l10n_file['xliff']} -o {l10n_file['ts']}")

print(f'Imported {len(l10n_files)} locales')
