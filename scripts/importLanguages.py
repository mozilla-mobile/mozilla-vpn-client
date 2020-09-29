#!/usr/bin/python3
# Note: this wont work on python 2.7
# This script must be executed at the root of the repository.

import xml.etree.ElementTree as ET
import os

THRESHOLD = 0.01 # Target Completeness for import

# Make sure the Target ts files are up to date
os.system(f'lupdate src/src.pro -ts')

for fileName in os.listdir('i18n'):
    if(not fileName.endswith(".xlf")):
        continue # Skip non xlf files
    language = fileName.split("_")[1].split(".")[0]

    tree = ET.parse('i18n/'+fileName)
    root = tree.getroot()
    
    elementCount = 0
    translations = 0 

    for element in root.iter("{urn:oasis:names:tc:xliff:document:1.2}target"):
        elementCount+=1
        if(element.text):
                translations+=1
    
    complete = translations/(elementCount*1.0)
    if(complete < THRESHOLD):
            print(f"❌\t- {language} is not completed {round(complete*100,2)}%, at least {THRESHOLD*100}% are needed")
            continue # Not enough translations next file please
    basename = fileName.split(".")[0]
    os.system(f'lconvert -i translations/{basename}.ts -i i18n/{basename}.xlf -o translations/{basename}.ts')
    print(f"✔\t- {language} imported to translations/{basename}.ts")