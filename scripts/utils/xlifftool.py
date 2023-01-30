#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import sys
import re
import xml.etree.ElementTree as ET

## XLIFF language class, holds the stringdb for a given locale
class xliff_language:
    def __init__(self, locale, verbose=False):
        self.__locale = locale.replace('_', '-')
        self.__stringdb = {}
        self.__verbose = verbose

    ## Read-only access to the locale.
    @property
    def locale(self):
        return self.__locale

    ## Returns the completeness of the translation as a ratio between 0.0 and 1.0
    @property
    def completeness(self):
        sources = len(self.__stringdb)
        translations = 0
        for x in self.__stringdb.values():
            if len(x['target']) != 0:
                translations += 1
        return translations / (sources * 1.0) if sources > 0 else 0.0

    ## Returns the list of known translation IDs.
    @property
    def keys(self):
        return self.__stringdb.keys()

    ## Given a translation ID, try out best to translate it into a printable string.
    def translate(self, trid):
        if trid not in self.__stringdb:
            return trid

        unit = self.__stringdb[trid]
        if len(unit['target']) != 0:
            return unit['target']
        elif len(unit['source']) != 0:
            return unit['source']
        else:
            return trid

    # Render a string into the desired format.
    def render(self, trid, format=None, strip_trid=0):
        if trid not in self.__stringdb:
            if self.__verbose:
                print(f'String ID {trid} was not found', file=sys.stderr)
            return

        target = self.translate(trid)

        while len(trid) > 0 and strip_trid > 0:
            index = trid.find('.')
            if index < 0:
                break
            strip_trid = strip_trid - 1
            trid = trid[index+1:]

        if format == 'env':
            escstring = target.replace('"', '\\"')
            return f"{trid.upper().replace('.', '_')}=\"{escstring}\""
        elif format == 'macos':
            escstring = target.replace('\'', '\\\'')
            return f"'{trid.lower().replace('.', '_')}' = '{escstring}';"
        elif format == 'text':
            return f"{trid}: {target}"
        else:
            return target
    
    # Perform variable substitution and return the transformed text.
    def transform(self, text):
        start = 0
        ac_regex = re.compile('@[\w.]+@') ## Autoconf style match: @VARNAME@
        cm_regex = re.compile('\${[\w.]+}')  ## CMake style match: ${VARNAME}
        qt_regex = re.compile('qtTrId("[\w.]+")') ## Qt match: qtTrId("VARNAME")

        while start < len(text):
            ## Search for the first thing that we can translate
            ac_match = ac_regex.search(text, start) 
            ac_start = ac_match.start() if ac_match else len(text)
            cm_match = cm_regex.search(text, start)
            cm_start = cm_match.start() if cm_match else len(text)
            qt_match = qt_regex.search(text, start)
            qt_start = qt_match.start() if qt_match else len(text)

            ## Determine if we matched something we can localize.
            if ac_match and (ac_start < cm_start) and (ac_start < qt_start):
                match = ac_match
                trid = ac_match.group(0)[1:-1]
            elif cm_match and (cm_start < ac_start) and (cm_start < qt_start):
                match = cm_match
                trid = cm_match.group(0)[2:-1]
            elif qt_match and (qt_start < ac_start) and (qt_start < cm_start):
                match = cm_match
                trid = qt_match.group(0)[8:-2] ## TODO: Won't handle whitespace well.
            else:
                break

            ## Replace the string with its translation, if found.
            string = None
            if trid in self.__stringdb:
                string = self.translate(trid)
            else:
                ## Also consider snake cases and environment variable forms.
                for key in self.__stringdb:
                    if (key.upper().replace('.', '_') != trid.upper()):
                        continue
                    string = self.translate(key)
                    break

            ## Replace the matched string and continue.
            if string is None:
                start = match.end()
            else:
                start = match.start() + len(string)
                text = text[0:match.start()] + string + text[match.end():]

        return text

    ## Parse an XLIFF <file> element for translation data.
    def parse_filegroup(self, elem):
        if elem.attrib['target-language'] == self.locale:
            ## Use the target strings
            for unit in elem.iter('{urn:oasis:names:tc:xliff:document:1.2}trans-unit'):
                trid = unit.attrib['id']
                self.__stringdb[trid] = {
                    'origin': elem.attrib['original'],
                    'source': unit.findtext('{urn:oasis:names:tc:xliff:document:1.2}source', default=''),
                    'target': unit.findtext('{urn:oasis:names:tc:xliff:document:1.2}target', default='')
                }
        elif elem.attrib['source-language'] == self.locale:
            ## Use the source strings
            for unit in elem.iter('{urn:oasis:names:tc:xliff:document:1.2}trans-unit'):
                trid = unit.attrib['id']
                self.__stringdb[trid] = {
                    'origin': elem.attrib['original'],
                    'source': unit.findtext('{urn:oasis:names:tc:xliff:document:1.2}source', default=''),
                    'target': unit.findtext('{urn:oasis:names:tc:xliff:document:1.2}source', default='')
                }

    ## Parse an XLIFF file for translation data.
    def parse_xliff(self, filename):
        tree = ET.parse(args.source)
        for filegroup in tree.getroot().iter('{urn:oasis:names:tc:xliff:document:1.2}file'):
            self.parse_filegroup(filegroup)

        ## Print a summary after parsing a file
        if self.__verbose:
            print(f'{self.completeness * 100.0:6.2f}  {self.locale:6}  {filename}', file=sys.stderr)

## Assuming that the XLIFF file only translates for a single language, try
## to guess what it is by finding the most common `target-language` attribute
def guess_locale(filename):
    tree = ET.parse(args.source)
    xlanguages = {}
    for filegroup in tree.getroot().iter('{urn:oasis:names:tc:xliff:document:1.2}file'):
        xlocale = filegroup.attrib['target-language']
        xlanguages[xlocale] = xlanguages.get(xlocale, 0) + 1

    best_count = -1
    best_locale = None
    for xlocale in xlanguages:
        if xlanguages[xlocale] > best_count:
            best_count = xlanguages[xlocale]
            best_locale = xlocale

    return best_locale

if __name__ == "__main__":
    ## Parse arguments to locate the input files and options.
    parser = argparse.ArgumentParser(
        formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(prog,max_help_position=32),
        description='Parse and extract data from XLIFF translation files',
        epilog="""\
Output format can take one of the following values:
   text      ID and string separated by colons
   env       Environment variable format
   macos     MacOS Localizable.strings file
   raw       Just the translation string
""")

    parser.add_argument('source', metavar='SOURCE', type=str, action='store',
        help='XLIFF translation file to parse')
    parser.add_argument('-v', '--verbose', default=False, action='store_true',
        help='Print verbose information about the translation file')

    ## Parser argument group
    parsegroup = parser.add_argument_group('Options controlling the parser')
    parsegroup.add_argument('-c', '--check', default=False, action='store_true',
        help='Check if the XLIFF file is ready for import')
    parsegroup.add_argument('-C', '--completeness', default=False, action='store_true',
        help='Print the completeness level of the XLIFF file')
    parsegroup.add_argument('-t', '--threshold', metavar='VAL', type=float, default=0.7,
        help='Minimum required threshold of completed translations (0.0 to 1.0)')
    parsegroup.add_argument('-l', '--locale', metavar='LANG', type=str, action='store',
        help='Find translations for the language LANG')

    ## Output argument group
    outgroup = parser.add_argument_group('Options controlling the output')
    outgroup.add_argument('-o', '--output', metavar='FILE', type=str,
        help='Write output to FILE (default: stdout)')
    outgroup.add_argument('-f', '--format', metavar='FMT', default='text',
        choices=['text', 'env', 'macos', 'raw'],
        help='Output format to write (default: text)')
    outgroup.add_argument('-g', '--get', metavar='TRID', type=str, action='append', default=[],
        help='Get translated strings matching TRID')
    outgroup.add_argument('-d', '--dump', action='append_const', dest='get', const='all',
        help='Dump all translations')
    outgroup.add_argument('-x', '--xform', metavar='FILE', type=str, action='store',
        help='Transform template text from FILE')
    outgroup.add_argument('--strip', metavar='NUM', type=int, default=0, action='store',
        help='Strip NUM segments from the translation ID when printing')
    args = parser.parse_args()

    # Parse the XLIFF
    if args.locale is None:
        args.locale = guess_locale(args.source)
    xlanguage = xliff_language(args.locale, args.verbose)
    xlanguage.parse_xliff(args.source)

    # Render the user's desired output
    with open(args.output, 'w', encoding='utf-8') if args.output else sys.stdout as fout:
        ## Request translation strings
        for trid in args.get:
            if trid == 'all':
                for trid in xlanguage.keys:
                    print(f"{xlanguage.render(trid, format=args.format, strip_trid=args.strip)}", file=fout)
            elif trid in xlanguage.keys:
                print(f"{xlanguage.render(trid, format=args.format, strip_trid=args.strip)}", file=fout)
            else:
                for id in [id for id in xlanguage.keys if id.startswith(trid + '.')]:
                    print(f"{xlanguage.render(id, format=args.format, strip_trid=args.strip)}", file=fout)

        ## Transform a file
        if args.xform:
            with open(args.xform, "r") as fin:
                for line in fin:
                    fout.write(xlanguage.transform(line))

        # Return the completeness value
        if args.completeness:
            print(xlanguage.completeness)

    # Return the verdict
    if args.check:
        if xlanguage.completeness < args.threshold:
            sys.exit(1)
