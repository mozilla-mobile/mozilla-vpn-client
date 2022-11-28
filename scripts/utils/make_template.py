#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This program attempts to emulate CMake's `configure_file()` command
# as closely as possible, but can be run as independetly from cmake as
# a script, or as a build command.

import sys
import re
import argparse

# Parse arguments to determine what to do.
parser = argparse.ArgumentParser(
    description='Generate a file from a template')
parser.add_argument('template', metavar='TEMPLATE', type=str, action='store',
    help='Template file to process')
parser.add_argument('-o', '--output', metavar='FILENAME', type=str, action='store',
    help='Output file to write')
parser.add_argument('-k', '--keyword', metavar='KEY=VALUE', type=str, action='append', default=[],
    help='Keyword to replace, and the value to replace it with')
parser.add_argument('-f', '--keyfile', metavar='KEY=FILE', type=str, action='append', default=[],
    help='Keyword to replace, and the file to source its value from')
args = parser.parse_args()

# Build up a dictionary of keywords and their replacement values
keywords = {}
for keyval in args.keyword:
    kvsplit = keyval.split("=", 1)
    if len(kvsplit) != 2:
        print('Unable to parse KEY=VALUE from: ' + keyval)
        sys.exit(1)
    keywords[kvsplit[0]] = kvsplit[1]

for keyfile in args.keyfile:
    kfsplit = keyfile.split("=", 1)
    if len(kfsplit) != 2:
        print('Unable to parse KEY=FILE from: ' + keyfile)
        sys.exit(1)
    with open(kfsplit[1]) as fp:
        keywords[kfsplit[0]] = fp.read()

# Scan through the string for each of the keywords, replacing them
# as they are found, while taking care not to apply transformations
# to any already-transformed text.
def transform(text):
    start = 0
    ac_regex = re.compile('@[\w.]+@') ## Autoconf style match: @VARNAME@
    cm_regex = re.compile('\${[\w.]+}') ## CMake style match: ${VARNAME}
    xc_regex = re.compile('\$\([\w.]+\)') ## Xcode style match: $(VARNAME)
    while start < len(text):
        ## Search for the next variable to substitute.
        ac_match = ac_regex.search(text, start)
        ac_start = ac_match.start() if ac_match else len(text)
        cm_match = cm_regex.search(text, start)
        cm_start = cm_match.start() if cm_match else len(text)
        xc_match = xc_regex.search(text, start)
        xc_start = xc_match.start() if xc_match else len(text)

        if ac_match and (ac_start < cm_start) and (ac_start < xc_start):
            match = ac_match
            name = ac_match.group(0)[1:-1]
        elif cm_match and (cm_start < ac_start) and (cm_start < xc_start):
            match = cm_match
            name = cm_match.group(0)[2:-1]
        elif xc_match and (xc_start < ac_start) and (xc_start < cm_start):
            match = xc_match
            name = xc_match.group(0)[2:-1]
        else:
            # If there are no matches, we can return
            break

        ## If there is no value, skip the match and keep searching.
        if name not in keywords:
            start = match.end()
            continue

        # Substitute the keyword and adjust the start.
        value = keywords[name]
        start = match.start() + len(value)
        text = text[0:match.start()] + value + text[match.end():]

    return text

# Open the output file
if args.output is None:
    fout = sys.stdout
else:
    fout = open(args.output, "w")

# Read through the input file and apply variable substitutions.
with open(args.template) as fin:
    fout.write(transform(fin.read()))

fout.flush()
fout.close()
