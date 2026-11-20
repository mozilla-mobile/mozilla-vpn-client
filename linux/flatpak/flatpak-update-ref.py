#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
import argparse
import os
import subprocess
import re
import sys

# Run a git command from same directory as this script and return its output.
def gitcmd(*args):
    path = os.path.dirname(os.path.realpath(__file__))
    result = subprocess.run(["git", *args], cwd=path, capture_output=True, encoding='utf8')
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    result.check_returncode()
    return result.stdout.strip()

if __name__ == "__main__":
    defaultServer = os.getenv('GITHUB_SERVER_URL', 'https://github.com')
    defaultRepository = os.getenv('GITHUB_REPOSITORY', 'mozilla-mobile/mozilla-vpn-client')

    ## Parse arguments for the control file and desired dependency section.
    parser = argparse.ArgumentParser(description='Update a flatpak manifest with a git ref')
    parser.add_argument('manifest', metavar='MANIFEST', type=str, action='store',
                        help='Flatpak manifest YAML to parse')
    parser.add_argument('-t', '--tag', metavar='TAG', type=str, action='store',
                        help='git tag to use sof rthe source ref')
    parser.add_argument('-b', '--branch', metavar='NAME', type=str, action='store',
                        help='git branch to use for the source ref')
    parser.add_argument('-c', '--commit', metavar='SHA', type=str, action='store',
                        help='git commit hash to use for the source ref')
    parser.add_argument('-u', '--url', metavar='URL', type=str, action='store',
                        default=f'{defaultServer}/{defaultRepository}.git',
                        help='URL of the git repository')
    args = parser.parse_args()

    # Autodetect the branch/tag to use from the github environment.
    if args.tag is None and args.branch is None:
        reftype = os.getenv('GITHUB_REF_TYPE')
        if reftype == "tag":
            args.tag = os.getenv("GITHUB_REF_NAME")
            args.commit = os.getenv("GITHUB_SHA")
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

    # Find a source starting with '- type: dir' and swap it out with a git ref.
    pattern = re.compile(r'\s+- type:\s+dir')
    with open(args.manifest, 'r') as fp:
        while True:
            line = fp.readline()
            if len(line) == 0:
                break
            line = line.rstrip()
            m = pattern.search(line)
            if (m is None) or (m.start() != 0):
                print(line)
                continue

            # Generate the updated tag.
            indent = line.split('-')[0]
            print(f'{indent}- type: git')
            print(f'{indent}  url: {args.url}')
            if args.tag:
                print(f'{indent}  tag: {args.tag}')
            elif args.branch:
                print(f'{indent}  branch: {args.branch}')
            print(f'{indent}  commit: {args.commit}')
            print('')

            # Read and discard until we find an empty line.
            while fp.readline().strip():
                pass
            break

        # Dump the remainder of the file to the output.
        while True:
            line = fp.readline()
            if len(line) == 0:
                break
            print(line.rstrip())
