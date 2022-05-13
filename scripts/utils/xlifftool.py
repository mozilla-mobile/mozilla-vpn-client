#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import sys
import xml.etree.ElementTree as ET

## Parse arguments to locate the input files and options.
parser = argparse.ArgumentParser(
    description='Inspect XLIFF translation files')
parser.add_argument('source', metavar='SOURCE', type=str, action='store',
    help='XLIFF translation file to process')
parser.add_argument('-c', '--check', default=False, action='store_true',
    help='Check if the XLIFF file is ready for import')
parser.add_argument('-g', '--get', metavar='TRID', type=str, action='append', default=[],
    help='Get translated string for TRID')
parser.add_argument('-t', '--threshold', metavar='VAL', type=float, default=0.7,
    help='Minimum required threshold of completed translations (0.0 to 1.0)')
parser.add_argument('-v', '--verbose', default=False, action='store_true',
    help='Print verbose information about the translation file')
args = parser.parse_args()

## Parse the input XLIFF file as XML and check its completeness.
def get_completeness(root):
    results = {}
    for filegroup in root.iter('{urn:oasis:names:tc:xliff:document:1.2}file'):
        target = filegroup.attrib['target-language']
        sources = 0
        translations = 0
        for element in filegroup.iter('{urn:oasis:names:tc:xliff:document:1.2}source'):
            sources += 1
        for element in filegroup.iter('{urn:oasis:names:tc:xliff:document:1.2}target'):
            translations += 1
        
        if target in results:
            sources += results[target]['sources']
            translations += results[target]['translations']
        
        results[target] = {
            'sources': sources,
            'translations': translations,
        }
    
    sources = 0
    translations = 0
    for target in results:
        sources += results[target]['sources']
        translations += results[target]['translations']
        if args.verbose:
            x = results[target]['sources']
            score = (results[target]['translations'] * 100.0) / x if x > 0 else 0.0
            print(f'{score:6.2f}  {target:6}  {args.source}')

    return translations / (sources * 1.0) if sources > 0 else 0.0

def get_translation(root, trid):
    for unit in root.iter("{urn:oasis:names:tc:xliff:document:1.2}trans-unit"):
        if unit.attrib['id'] != trid:
            continue

        string = unit.find("{urn:oasis:names:tc:xliff:document:1.2}target")
        if string is not None:
            return string.text

        string = unit.find("{urn:oasis:names:tc:xliff:document:1.2}source")
        if string is not None:
            return string.text

    return None

# Parse the XLIFF
tree = ET.parse(args.source)
xliffroot = tree.getroot()

# Return translations
for trid in args.get:
    translation = get_translation(xliffroot, trid)
    if translation is None:
        print(f"Translation for {trid} was not found.", file=sys.stderr)
        sys.exit(1)
    else:
        print(f"{translation}")

# Return the verdict
if args.check:
    if get_completeness(xliffroot) < args.threshold:
        sys.exit(1)
