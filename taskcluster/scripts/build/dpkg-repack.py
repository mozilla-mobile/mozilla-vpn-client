#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import subprocess
import os
import tempfile
import re
import sys
from collections import OrderedDict

def convert_case(name):
    """Convert strings to Deb822-style case"""
    start = True
    result = ''
    for char in name:
        if not char.isalnum():
            result += '-'
            start = True
        elif start:
            result += char.upper()
            start = False
        else:
            result += char.lower()
            start = False
    return result

def parse_control(filename):
    """Parse a Debian package control file into an OrderedDict"""
    contents = OrderedDict()
    restart = re.compile('^[A-Z][A-Za-z0-9-]+:')

    with open(filename, 'r') as fp:
        section = None
        for line in fp.readlines():
            line = line.rstrip()
            if restart.match(line):
                section, text = line.split(':', 1)
                section = convert_case(section)
                contents[section] = text
            elif section is not None:
                contents[section] = contents[section] + '\n' + line

    return contents

def write_control(contents, file=sys.stdout):
    """Print an OrderedDict as a Debian package control file"""
    for key in contents:
        print(f"{key}: {contents[key]}", file=file)

if __name__ == "__main__":
    argparser = argparse.ArgumentParser(
        description='Repack a Debian binary package while modifying its control file')

    argparser.add_argument('filename', metavar='INPUT', type=str,
        help='Debian package to repack')
    argparser.add_argument('-o', '--output', metavar='OUTPUT', type=str,
        help='Output debian package file')
    argparser.add_argument('-s', '--set', metavar='KEY=VALUE', type=str, action='append', default=[],
        help='Set or update values in the package control file')
    args = argparser.parse_args()

    # Default output behavior modifies the package in place.
    if not args.output:
        args.output = args.filename

    with tempfile.TemporaryDirectory() as tempdir:
        # Unpack the Debian package.
        archivedir = os.path.join(tempdir, 'archive')
        os.mkdir(archivedir)
        subprocess.check_call(['ar', 'x', f'--output={archivedir}', args.filename])

        # Unpack the control files.
        # TODO: Support other compression algorithms besides gzip?
        controldir = os.path.join(tempdir, 'control')
        os.mkdir(controldir)
        subprocess.check_call(['tar', '-C', controldir, '-xf', os.path.join(archivedir, 'control.tar.gz')])

        # Parse the control file
        contents = parse_control(os.path.join(controldir, 'control'))

        # Set/update extra values in the control file
        for keyval in args.set:
            key, value = keyval.split('=', 1)
            contents[convert_case(key)] = value

        # Ensure the 'Description' always goes last and then write the updated control file.
        contents.move_to_end('Description')
        write_control(contents, file=sys.stderr)
        with open(os.path.join(controldir, 'control'), 'w') as fp:
            write_control(contents, file=fp)

        # Repack the Debian package files.
        print(f"Writing updated package to {os.path.abspath(args.output)}", file=sys.stderr)
        subprocess.check_call(['tar', '-C', controldir, '-cf', os.path.join(archivedir, 'control.tar.gz')] + os.listdir(controldir))
        subprocess.check_call(['ar', 'ru', os.path.abspath(args.output)] + os.listdir(archivedir), cwd=archivedir)
