#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import json
import os
import sys
import shutil
import subprocess

# Parse arguments to determine what to do.
parser = argparse.ArgumentParser(description='Merge two more or more plist files')
parser.add_argument('input', metavar='INFILE', type=str, nargs='+',
    help='Template file to process')
parser.add_argument('-o', '--output', metavar='DST', type=str, action='store',
    help='Output file to write or update')
parser.add_argument('-f', '--format', metavar='FMT', type=str, action='store',
    help='Output file format', default='binary1')
args = parser.parse_args()

# Find our tools
plutil = shutil.which('plutil')
if plutil is None:
    print(f'Failed to locate plutil tool', file=sys.stderr)
    sys.exit(1)
plbuddy = shutil.which('PlistBuddy', path='/usr/libexec')
if plbuddy is None:
    print(f'Failed to locate PlistBuddy tool', file=sys.stderr)
    sys.exit(1)

# Extract the top level keys from a .plist file.
def extractkeys(filename):
    result = subprocess.run([plutil, '-convert', 'json', '-o', '-', filename],
                            stdout=subprocess.PIPE, check=True)

    js = json.loads(result.stdout)
    if not isinstance(js, dict):
        print(f'Failed to parse input plist from {filename}', file=sys.stderr)
        os.exit(1)

    return list(js.keys())

print(f'Merging content into {args.output}')
topkeys = []
if os.path.isfile(args.output):
    # Parse the output file for the list of known keys.
    topkeys = extractkeys(args.output)
else:
    # If the output file doesn't exist, create an empty plist file.
    subprocess.call([plutil, '-create', args.format, args.output])

# Launch PlistBuddy to manipulate the output file.
proc = subprocess.Popen([plbuddy, args.output], encoding='utf-8',
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=sys.stderr)

# Merge the input files into the output.
for infile in args.input:
    # Drop duplicate keys and update topkeys.
    print(f'Merging content from {infile}')
    for keyname in extractkeys(infile):
        print(f'Replacing key: {keyname}')
        if keyname in topkeys:
            print(f'Delete :{keyname}', file=proc.stdin)
        else:
            topkeys.append(keyname)

    # Merge the new content.
    print(f'Merge "{infile}"', file=proc.stdin)

# Write the changes to the file.
print(f'Save', file=proc.stdin)
print(f'Exit', file=proc.stdin)
proc.stdin.close()
proc.wait()

# Convert the plist back into the desired output format, since PlistBuddy
# almost always produces XML output.
subprocess.run([plutil, '-convert', args.format, args.output], check=True)
