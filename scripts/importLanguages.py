#!/usr/bin/python3
# Note: this wont work on python 2.7
# This script must be executed at the root of the repository.

import xml.etree.ElementTree as ET
import os

THRESHOLD = 0.70 # 70% Target Completeness for import
TS_FILES = []
# Make sure the Target ts files are up to date
os.system(f'lupdate src/src.pro -ts')

for path in os.listdir('i18n'):
    if(not os.path.isdir(f'i18n/{path}')):
        continue # Skip non dirs
    if not len(path.split("-")[0]) == 2: # All language folders are format ab(-cd)/
        continue # Skip folders that not have isoLang format
    language = path
    filePath = f'i18n/{language}/mozillavpn.xlf'
    tree = ET.parse(filePath)
    root = tree.getroot()
    
    elementCount = 0
    translations = 0 

    for element in root.iter("{urn:oasis:names:tc:xliff:document:1.2}source"):
        elementCount+=1
    for element in root.iter("{urn:oasis:names:tc:xliff:document:1.2}target"):
        if(element.text):
                translations+=1
    
    complete = translations/(elementCount*1.0)
    if(complete < THRESHOLD):
            print(f"❌\t- {language} is not completed {round(complete*100,2)}%, at least {THRESHOLD*100}% are needed")
            continue # Not enough translations next file please
    basename = f'mozillavpn_{language}.ts'
    os.system(f'lconvert -i translations/{basename} -i {filePath} -o translations/{basename}')
    print(f"✔\t- {language} imported to translations/{basename}")
    TS_FILES.append(f"translations/{basename}")

# Write PRI file to import 
projectFile = open("translations/translations.pri", "w")
projectFile.write("TRANSLATIONS += \ \n")
for path in TS_FILES:
    projectFile.write(f"{TS_FILES} \ \n")
projectFile.write("\n \n ##End")
projectFile.close()