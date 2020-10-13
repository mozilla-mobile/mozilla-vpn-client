#!/usr/bin/python3
# Note: this wont work on python 2.7
# This script must be executed at the root of the repository.

import xml.etree.ElementTree as ET
import os

THRESHOLD = 0.70 # 70% Target Completeness for import
FILES = []

# Step 1 - 
# go through the i18n repo, check each xlf file and take 
# note which lang is >70% complete
# Adds path of {.xlf / .ts } to FILES
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
    basename = f'mozillavpn_{language}'
    print(f"✔\t- {language} added")
    FILES.append({
       "ts" : f"translations/{basename}.ts",
       "xlf": filePath
    })

if len(FILES) == 0:
    print("No Languages were imported")
    os.system(f'lupdate src/src.pro -ts translations/mozillavpn_en.ts')
    exit(0)

# Step 2 -  
# Write PRI file to import the done languages 
projectFile = open("translations/translations.pri", "w")
projectFile.write("TRANSLATIONS += \ \n")
for file in FILES:
    projectFile.write(f"../{file['ts']} \ \n")
projectFile.write("\n \n ##End")
projectFile.close()
print("Updated translations.pri")


# Step 3 - generate new ts files
os.system(f'lupdate src/src.pro -ts')
# Step 4 - now import done translations into the files
for file in FILES:
    os.system(f"lconvert -i {file['ts']} -i {file['xlf']} -o {file['ts']}")
print("Imported Languages")
