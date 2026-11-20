#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
import argparse
from datetime import date
import os
import sys
import xml.etree.ElementTree as etree

if __name__ == "__main__":
    defaultServer = os.getenv('GITHUB_SERVER_URL', 'https://github.com')
    defaultRepository = os.getenv('GITHUB_REPOSITORY', 'mozilla-mobile/mozilla-vpn-client')

    ## Parse arguments for the control file and desired dependency section.
    parser = argparse.ArgumentParser(description='Update a release manifest with a release tag')
    parser.add_argument('manifest', metavar='MANIFEST', type=str, action='store',
                        help='Release manifest XML to parse')
    parser.add_argument('-t', '--tag', metavar='TAG', type=str, action='store',
                        default=os.getenv('GITHUB_REF_NAME'),
                        help='git tag to use sof rthe source ref')
    parser.add_argument('-r', '--repo', metavar='URL', type=str, action='store',
                        default=f'{defaultServer}/{defaultRepository}',
                        help='URL of the git repository')
    args = parser.parse_args()

    # Autodetect the tag to use from the github environment.
    if args.tag is None:
        print("Unable to determine releaset tag", file=sys.stderr)
        sys.exit(1)

    # Parse the release XML file.
    tree = etree.parse(args.manifest)
    root = tree.getroot()
    indent = root.text if root.text.isspace() else '\n  '

    # Prepare the release URL from the tag.
    version = args.tag[1:] if (args.tag[0] == 'v') else args.tag
    version = version.replace('-', '~')
    details = {
        "version": version,
        "date": date.today().isoformat(),
        "type": "development"
    }
    entry = etree.Element('release', attrib=details)
    entry.text = f'{indent}  '
    entry.tail = indent
    url = etree.SubElement(entry, 'url', attrib={"type": "details"})
    url.text = f'{args.repo}/releases/tag/{args.tag}'
    url.tail = indent

    # Insert the new version into the start of the release manifest.
    root.insert(0, entry)
    tree.write(args.manifest)
