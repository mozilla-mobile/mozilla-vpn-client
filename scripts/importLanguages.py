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

# Include only locales above this threshold (e.g. 70%) in production
l10n_threshold = 0.70

parser = argparse.ArgumentParser()
parser.add_argument(
    '-p', '--prod', default=False, action="store_true", dest="isprod",
    help='Build only for production locales.')
args = parser.parse_args()

# Step 0
# Locate the lupdate and lconvert tools
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
            'ts': os.path.join('translations', f'mozillavpn_en.ts'),
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

    baseName = f'mozillavpn_{locale}'
    print(f'OK\t- {locale} added ({round(completeness*100, 2)}% translated)')
    l10n_files.append({
        'ts': os.path.join('translations', f'{baseName}.ts'),
        'xliff': xliff_path
    })

# Step 2
# Write PRI file to import the locales that are ready
with open('translations/translations.pri', 'w') as pri_file:
    output = []
    output.append('TRANSLATIONS += \\ ')
    for file in l10n_files:
        output.append(f"../{file['ts']} \\ ")
    output.append('\n \n##End')
    pri_file.write('\n'.join(output))
print('Updated translations.pri')

# Step 3
# Generate new ts files
os.system(f"{lupdate} src/src.pro")

# Step 4
# Now merge translations into the files
for l10n_file in l10n_files:
    os.system(f"{lconvert} -i {l10n_file['ts']} -if xlf -i {l10n_file['xliff']} -o {l10n_file['ts']}")

print(f'Imported {len(l10n_files)} locales')
