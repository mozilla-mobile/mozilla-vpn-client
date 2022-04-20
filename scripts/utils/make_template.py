#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import sys
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
    while start < len(text):
        # Find the next matching keyword, if any.
        matchIdx = -1
        matchKey = ""
        for key in keywords:
            x = text.find(key, start)
            if (matchIdx < 0) or (x < matchIdx):
                matchIdx = x
                matchKey = key
        
        # If there are no matches, we can return.
        if matchIdx < 0:
            return text
        
        # Substitute the keyword and adjust the start.
        value = keywords[matchKey]
        start = matchIdx + len(value)
        text = text[0:matchIdx] + value + text[matchIdx+len(matchKey):]

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
