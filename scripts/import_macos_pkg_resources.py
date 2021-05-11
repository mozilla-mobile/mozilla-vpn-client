#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script must be executed at the root of the repository.

import os
import xml.etree.ElementTree as ET

# List of translation keys
keys = [ "macosinstaller.title",
         "macosinstaller.unsupported_version.title",
         "macosinstaller.unsupported_version.message",
         "macosinstaller.previous_build.title",
         "macosinstaller.previous_build.message",
         "macosinstaller.welcome.message1",
         "macosinstaller.welcome.message2",
         "macosinstaller.conclusion.title",
         "macosinstaller.conclusion.message1",
         "macosinstaller.conclusion.message2",
         "macosinstaller.conclusion.message3" ];

# Simple method that retrieves the text value of a localized string.
def getTranslation(root, string, nodeName):
    for node in root.findall(f"./context/message[@id='{string}']"):
        for child in node:
            if child.tag != nodeName:
                continue
            if child.text == None:
                return ''
            return child.text
    return ''

# Generate the translation folder for a particular locale. `nodeTag` can be
# `source` (for english only) or `translation` (for anything else)
def translate(root, locale, nodeTag):
  folder = os.path.join('macos', 'pkg', 'Resources', locale + '.lproj')
  os.mkdir(folder)

  localizableFile = os.path.join(folder, 'Localizable.strings')
  f = open(localizableFile, 'w')
  for key in keys:
      f.write(f"'{key.replace('macosinstaller.', '').replace('.', '_')}' = '{getTranslation(root, key, nodeTag)}';\n")
  f.close()

  welcomeFile = os.path.join(folder, "welcome.html")
  f = open(welcomeFile, 'w')
  f.write('<!DOCTYPE html>\n<html lang="en">\n<head>\n<meta charset="utf-8" />\n</head>')
  f.write('<body style="color: #1a1919; line-height: 1.5; font-family: -apple-system, BlinkMacSystemFont, \'Helvetica\', sans-serif; font-size: 13px; ">\n')
  f.write('<br />\n')
  f.write(f'<p style="margin: 5px;">{getTranslation(root, "macosinstaller.welcome.message1", nodeTag)}</p>\n');
  f.write('<br />\n')
  f.write(f'<p style="margin: 5px;">{getTranslation(root, "macosinstaller.welcome.message2", nodeTag)}</p>\n');
  f.write('</body>\n</html>')
  f.close()

  conclusionFile = os.path.join(folder, "conclusion.html")
  f = open(conclusionFile, 'w')
  f.write('<!DOCTYPE html>\n<html lang="en">\n<head>\n<meta charset="utf-8" />\n</head>')
  f.write('<body style="color: #1a1919; line-height: 1.5; font-family: -apple-system, BlinkMacSystemFont, \'Helvetica\', sans-serif; font-size: 13px;">\n')
  f.write('<br />\n')
  f.write(f'<h3 style="margin-left: 5px; margin-right: 5px;">{getTranslation(root, "macosinstaller.conclusion.title", nodeTag)}</h3>\n');
  f.write(f'<p style="margin: 5px;">{getTranslation(root, "macosinstaller.conclusion.message1", nodeTag)}</p>\n')
  f.write('<br />\n')
  f.write(f'<p style="margin: 5px;">{getTranslation(root, "macosinstaller.conclusion.message2", nodeTag)}\n');
  f.write(f'<a rel="noopener noreferrer" href="https://support.mozilla.org/products/firefox-private-network-vpn?utm_source=mozilla-vpn&utm_medium=mozilla-vpn-installer&utm_campaign=mac-installer" style="color: #0a84ff;">{getTranslation(root, "macosinstaller.conclusion.message3", nodeTag)}</a></p>\n')
  f.write('</body>\n</html>')
  f.close()

# Let's check if the `macos/pkg/Resources` folder exists.
resourceFolder = os.path.join('macos', 'pkg', 'Resources')
if not os.path.isdir(resourceFolder):
    print(f"Folder '{resourceFolder}' should exist!")
    exit(1)

# Let's return an error if there are existing `lproj` folders.
for locale in os.listdir(resourceFolder):
    localeFile = os.path.join(resourceFolder, locale)
    if os.path.isfile(localeFile):
        continue

    if not locale.endswith('.lproj'):
        continue

    print(f"Unexpected folder '{localeFile}'.")
    exit(1)

# Checking the `translations` folder.
if not os.path.isdir('translations'):
    print("Folder 'translations' should exist!")
    exit(1)

# For each translation, lets see if it's completed. If yes, let's create the
# corresponding `Resources` folder.
for translation in os.listdir('translations'):
    translationFile = os.path.join('translations', translation)
    if not os.path.isfile(translationFile):
        continue

    if not translation.startswith('mozillavpn_'):
        continue

    tree = ET.parse(translationFile)
    root = tree.getroot()

    fullTranslated = True
    for key in keys:
        fullTranslated = getTranslation(root, key, 'translation') != ''
        if not fullTranslated:
            break
    if not fullTranslated:
        continue

    locale = translation.split(".")[0]
    locale = locale.split("mozillavpn_")[1]
    translate(root, locale, 'translation')

# Finally, the english translation.
translationFile = os.path.join('translations', 'mozillavpn_en.ts')
if not os.path.isfile(translationFile):
    print(f"Translation '{translationFile}' is missing! Have you imported the languages?")
    exit(1)

tree = ET.parse(translationFile)
root = tree.getroot()
translate(root, 'en', 'source')
