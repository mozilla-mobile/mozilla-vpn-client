#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
import argparse
import os
import json
import requests
import subprocess
import sys
import yaml
import xml.etree.ElementTree as etree

from datetime import date
from yaml.loader import SafeLoader

# Gets the directory of this python script.
def srcdir():
    return os.path.dirname(os.path.realpath(__file__))

# A YAML loader that keeps track of the line numbers
class SafeLineLoader(SafeLoader):
    def construct_mapping(self, node, deep=False):
        mapping = super(SafeLineLoader, self).construct_mapping(node, deep=deep)
        mapping['__line__'] = node.start_mark.line
        return mapping

# Run a git command from same directory as this script and return its output.
def gitcmd(*args):
    result = subprocess.run(["git", *args], cwd=srcdir(), capture_output=True, encoding='utf8')
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    result.check_returncode()
    return result.stdout.strip()

if __name__ == "__main__":
    defaultServer = os.getenv('GITHUB_SERVER_URL', 'https://github.com')
    defaultRepository = os.getenv('GITHUB_REPOSITORY', 'mozilla-mobile/mozilla-vpn-client')

    ## Parse arguments for the the release details
    parser = argparse.ArgumentParser(description='Update a flatpak manifest with a git ref')
    parser.add_argument('-t', '--tag', metavar='TAG', type=str, action='store',
                        help='git tag to use sof rthe source ref')
    parser.add_argument('-b', '--branch', metavar='NAME', type=str, action='store',
                        help='git branch to use for the source ref')
    parser.add_argument('-c', '--commit', metavar='SHA', type=str, action='store',
                        help='git commit hash to use for the source ref')
    parser.add_argument('-m', '--manifest', metavar='MANIFEST', type=str, action='store',
                        default=os.path.join(srcdir(), 'org.mozilla.vpn.yml'),
                        help='Flatpak manifest YAML to parse')
    parser.add_argument('-o', '--outdir', metavar='DEST', type=str, action='store',
                        default=os.getcwd(),
                        help='Output directory for flatpak release manifest')
    parser.add_argument('-u', '--url', metavar='URL', type=str, action='store',
                        default=f'{defaultServer}/{defaultRepository}',
                        help='URL of the git repository')
    args = parser.parse_args()

    # Autodetect the branch or tag to use from the github environment.
    if args.tag is None and args.branch is None:
        reftype = os.getenv('GITHUB_REF_TYPE')
        event = os.getenv('GITHUB_EVENT_NAME', '')
        if reftype == "tag":
            args.tag = os.getenv("GITHUB_REF_NAME")
            args.commit = os.getenv("GITHUB_SHA")
        elif event.startswith("pull_request"):
            args.branch = os.getenv("GITHUB_HEAD_REF")
            with open(os.getenv("GITHUB_EVENT_PATH"), 'r') as fp:
                webhook = json.load(fp)
                args.commit = webhook["pull_request"]["head"]["sha"]
        elif reftype == "branch":
            args.branch = os.getenv("GITHUB_REF_NAME")
            args.commit = os.getenv("GITHUB_SHA")
        else:
            args.branch = gitcmd("branch", "--show-current")

    # Autodetect the commit to use.
    if args.commit is None:
        if args.tag:
            args.commit = gitcmd("rev-parse", args.tag)
        else:
            args.commit = gitcmd("rev-parse", args.branch)

    # Load the manifest YAML
    with open(args.manifest, 'r') as fp:
        data = fp.read()
        module = yaml.load(data, Loader=SafeLineLoader)["modules"][-1]

    os.makedirs(args.outdir, exist_ok=True)

    # Generate an updated manifest YAML
    with open(os.path.join(args.outdir, 'org.mozilla.vpn.yml'), 'w') as fp:
        lines = data.splitlines()
        source = module["sources"][0]

        # Copy everything up to the start of the source.
        for i in range(source["__line__"]):
            print(lines.pop(0), file=fp)

        # Generate the source pointing to the branch or tag.
        indent = lines[0].split('-')[0]
        print(f'{indent}- type: git', file=fp)
        print(f'{indent}  url: {args.url}.git', file=fp)
        if args.tag:
            print(f'{indent}  tag: {args.tag}', file=fp)
        elif args.branch:
            print(f'{indent}  branch: {args.branch}', file=fp)
        print(f'{indent}  commit: {args.commit}', file=fp)
        print('', file=fp)

        # Generate a source to replace the releases manifest.
        print(f'{indent}- type: file', file=fp)
        print(f'{indent}  path: org.mozilla.vpn.releases.xml', file=fp)
        print(f'{indent}  dest: linux/extra', file=fp)
        print(f'{indent}  dest-filename: org.mozilla.vpn.releases.xml.in', file=fp)
        print('', file=fp)

        # Discard until we find an empty line, then dump the remainder
        discard = True
        while len(lines):
            i = lines.pop(0)
            if len(i) == 0:
                discard = False
            elif not discard:
                print(i, file=fp)

    # Download the latest releases XML data.
    r = requests.get('https://mozilla.github.io/mozillavpn-product-details/org.mozilla.vpn.releases.xml')
    r.raise_for_status()
    root = etree.fromstring(r.text)
    indent = root.text if root.text.isspace() else 'foo'

    # Append a development release to the XML data.
    if args.tag:
        version = args.tag[1:] if args.tag[0] == 'v' else args.tag
        version = version.replace('-', '~')
        details = {
            "version": version,
            "date": date.today().isoformat(),
            "type": "development"
        }
        release = etree.Element('release', attrib=details)
        release.tail = indent
        release.text = f'{indent}  '

        url = etree.SubElement(release, 'url', attrib={"type": "details"})
        url.text = f'{args.url}/releases/tag/{args.tag}'
        url.tail = indent
    else:
        filename = os.path.join(gitcmd('rev-parse', '--show-toplevel'), 'version.txt')
        with open(filename, 'r') as fp:
            version = f'{fp.read().strip()}~git{args.commit[0:8]}'
        details = {
            "version": version,
            "date": date.today().isoformat(),
            "type": "snapshot"
        }
        release = etree.Element('release', attrib=details)
        release.tail = indent

    root.insert(0, release)
    tree = etree.ElementTree(root)
    tree.write(os.path.join(args.outdir, 'org.mozilla.vpn.releases.xml'))
