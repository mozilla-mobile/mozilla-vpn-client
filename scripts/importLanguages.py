#! /usr/bin/env python3
# Note: this wont work on python 2.7
# This script must be executed at the root of the repository.

import xml.etree.ElementTree as ET
import os

THRESHOLD = 0.70  # 70% Target Completeness for import
FILES = []

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

if len(FILES) == 0:
    print('No Languages were imported')
    os.system(f'lupdate src/src.pro -ts translations/mozillavpn_en.ts')
    exit(0)

# Step 2
# Write PRI file to import the done languages
with open('translations/translations.pri', 'w') as projectFile:
    projectFile.write('TRANSLATIONS += \\ \n')
    for file in FILES:
        projectFile.write(f"../{file['ts']} \\ \n")
    projectFile.write('\n \n ##End')
print('Updated translations.pri')

# Step 3
# Generate new ts files
os.system(f'lupdate src/src.pro -ts')

# Step 4
# Now import done translations into the files
for file in FILES:
    os.system(f"lconvert -i {file['ts']} -if xlf -i {file['xliff']} -o {file['ts']}")
print('Imported Languages')