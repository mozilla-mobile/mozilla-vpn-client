#!/usr/bin/env python3
from debian.deb822 import Deb822
import argparse
import os

## Parse arguments for the control file and desired dependency section.
parser = argparse.ArgumentParser(description='List package dependencies')
parser.add_argument('control', type=str, nargs='?',
                    help='Debian control file to parse')
parser.add_argument('-b', '--build', action='store_true',
                    help='List packages from the Build-Depends paragraph')
parser.add_argument('-r', '--runtime', action='store_true',
                    help='List packages from the Depends paragraph')
parser.add_argument('-a', '--all', action='store_true',
                    help='List all packages dependencies')
args = parser.parse_args()

## If no control file was provided, assume control.qt6 from the source checkout.
if args.control is None:
    linuxdir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../linux/debian")
    args.control = os.path.normpath(os.path.join(linuxdir, "control.qt6"))

## Figure out which paragraphs to dump.
if args.all:
    args.build = True
    args.runtime = True

## Default to the runtime dependencies if nothing else is set.
if not (args.build or args.runtime):
    args.runtime = True

# Parse the control file to dump the package dependencies.
def dump(depends):
    for dep in depends.split(','):
        print(dep.split()[0])

for p in Deb822.iter_paragraphs(open(args.control)):
    for item in p.items():
        if args.runtime and item[0] == 'Depends':
            dump(item[1])
        if args.build and item[0] == 'Build-Depends':
            dump(item[1])

