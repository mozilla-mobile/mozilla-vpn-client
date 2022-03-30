#!/usr/bin/env python3
from debian.deb822 import Deb822
import argparse
import os

## Parse arguments for the control file
parser = argparse.ArgumentParser(description='List package dependencies')
parser.add_argument('control', type=str, nargs='?',
                    help='Debian control file to parse')
args = parser.parse_args()

## If no control file was provided, assume control.qt6 from the source checkout.
if args.control is None:
    linuxdir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../linux/debian")
    args.control = os.path.normpath(os.path.join(linuxdir, "control.qt6"))

# Parse the control file to dump the runtime dependencies.
for p in Deb822.iter_paragraphs(open(args.control)):
    for item in p.items():
        if item[0] != 'Depends':
            continue
        for dep in item[1].split(','):
            print(dep.split()[0])
