#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script must be executed at the root of the repository.

import xml.etree.ElementTree as ET
import os
import sys

THRESHOLD = 0.70  # 70% Target Completeness for import
FILES = []

PROD = False
PROD_LANGS = ["en"]

if len(sys.argv) > 1 and (sys.argv[1] == "-p" or sys.argv[1] == "--prod"):
    PROD = True

# Step 1
# Go through the i18n repo, check each xliff file and take
# note which locale is >70% complete
# Adds path of {.xliff / .ts } to FILES
for locale in os.listdir('i18n'):
    # Skip non folder
    if not os.path.isdir(os.path.join('i18n', locale)):
        continue
    # Skip hidden folders
    if locale.startswith('.'):
        continue

    if PROD and not(locale in PROD_LANGS):
        continue

    filePath = os.path.join('i18n', locale, 'mozillavpn.xliff')
    tree = ET.parse(filePath)
    root = tree.getroot()

    elementCount = 0
    translations = 0

    for element in root.iter('{urn:oasis:names:tc:xliff:document:1.2}source'):
        elementCount += 1
    for element in root.iter('{urn:oasis:names:tc:xliff:document:1.2}target'):
        if element.text:
            translations += 1

    complete = translations/(elementCount*1.0)
    if complete < THRESHOLD:
        print(f'❌\t- {locale} is translated at {round(complete*100,2)}%, at least {THRESHOLD*100}% is needed')
        continue  # Not enough translations next file please
    baseName = f'mozillavpn_{locale}'
    print(f'✔\t- {locale} added')
    FILES.append({
        'ts': os.path.join('translations', f'{baseName}.ts'),
        'xliff': filePath
    })

if len(FILES) == 0 or len([x for x in FILES if x['ts'] == os.path.join('translations', 'mozillavpn_en.ts')]) == 0:
    print('No fallback language (en) was imported')
    os.system('lupdate src -no-obsolete -ts translations/mozillavpn_en.ts')
    FILES.append({
        'ts': os.path.join('translations', 'mozillavpn_en.ts')
    })

# Step 2
# Write PRI file to import the done languages
with open('translations/translations.pri', 'w') as projectFile:
    projectFile.write('TRANSLATIONS += \\ \n')
    for file in FILES:
        projectFile.write(f"../{file['ts']} \\ \n")
        os.system(f"touch {file['ts']}")
    projectFile.write('\n \n##End')
print('Updated translations.pri')

# Step 3
# Generate new ts files
os.system('lupdate src/src.pro')

# Step 4
# Now import done translations into the files
for file in FILES:
    if 'xliff' in file.keys():
        os.system(f"lconvert -if xlf -i {file['xliff']} -o {file['ts']}")

print('Imported Languages')
