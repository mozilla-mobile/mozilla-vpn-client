#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import os

## Parse header files and output a list of test class declarations.
## This whole script could be an awk one-liner, but we can't be sure
## that awk exists on Windows, so we will do it in python instead.
parser = argparse.ArgumentParser(description='List test class declarations')
parser.add_argument('files', metavar='FILE', type=str, nargs='+',
                    help='C++ header files to parse for test classes')
parser.add_argument('-p', '--parent', metavar='NAME', type=str,
                    default='QObject',
                    help='Parent class name to match')
args = parser.parse_args()

## Parse a header file looking for class declarations
def parseFile(filename, parent):
    with open(filename) as fp:
        for line in fp:
            tokens = line.split(':')
            if len(tokens) != 2:
                continue

            ## Look for class delcarations
            declare = tokens[0].split()
            if len(declare) < 2:
                continue
            if declare[0] != 'class':
                continue

            ## Look for classes that inherit from the parent.
            inherit = tokens[1].replace(',', ' ').split()
            if inherit[-1] != '{':
                continue
            if not parent in inherit:
                continue

            ## We found a match, print it!
            print(declare[1])

for filename in args.files:
    parseFile(filename, args.parent)
