#! /usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import datetime
import os
import sys
import subprocess
import requests
import re

from dateutil import parser as dateparse
from packaging.version import Version
from urllib.parse import urlparse
from xml.etree import ElementTree

GITHUB_TOKEN = os.environ.get('GITHUB_TOKEN')

class release_info:
    def __init__(self, owner, project, tag):
        headers = {
            'Accept': 'application/vnd.github+json',
            'X-GitHub-Api-Version': '2022-11-28',
        }
        if GITHUB_TOKEN is not None:
            headers['Authorization'] = f"Bearer {GITHUB_TOKEN}"

        self.owner = owner
        self.project = project
        self.tag = tag

        # Fetch the release by tag
        release_url = f"https://api.github.com/repos/{owner}/{project}/releases/tags/{tag}"
        response = requests.get(release_url, headers=headers)
        if response.status_code == 200:
            # Parse the release info directly.
            self.parse_release(response.json())
        else:
            # If no such release exists - maybe we can get some useful info from the commit.
            commit_url = f"https://api.github.com/repos/{owner}/{project}/commits/tags/{tag}"
            self.parse_commit(requests.get(commit_url, headers=headers).json())

    def parse_release(self, js):
        self.details_url = js['html_url']
        self.description = js['body']
        self.timestamp = dateparse.parse(js['created_at'])

    def parse_commit(self, js):
        self.details_url = None
        self.description = None
        self.timestamp = dateparse.parse(js['commit']['committer']['date'])

    def __str__(self):
        return f"{self.tag}"
    
    def __repr__(self):
        return f"release_info({self.owner}, {self.project}, {self.tag})"

    @property
    def date(self):
        return self.timestamp.date()

    def toxml(self):
        attrs = {
            'version': self.tag,
            'date': self.date.isoformat(),
        }
        xml = ElementTree.Element('release', attrib=attrs)

        # Add the details URL
        if self.details_url:
            xurl = ElementTree.Element('url', attrib={'type': 'details'})
            xurl.text = self.details_url
            xml.append(xurl)

        # Add the description
        if self.description:
            xdesc = ElementTree.Element('description')
            xdesc.text = self.description
            xml.append(xdesc)

        return xml

def fetch_releases(gitremote, limit=8):
    # Parse apart the URL - which ought to point to a github project.
    url = urlparse(gitremote)
    if str(url.netloc) != 'github.com':
        print(f"Unsupported git host: {url.netloc}", file=sys.stderr)
        exit(1)
    user, project = url.path.strip('/').split('/')[:2]
    if project.endswith('.git'):
        project = project[:-4]

    # List all tags on the remote
    regex = re.compile(r'^refs/tags/(v[0-9]+\.[0-9]+\.[0-9]+)$')
    output = subprocess.check_output(['git', 'ls-remote', '--tags', gitremote]).decode('utf-8')
    tags = []
    for x in output.splitlines():
        sha, ref = x.split()
        match = regex.match(ref)
        if match is not None:
            tags.append(match.groups()[0])
    tags.sort(key=Version, reverse=True)

    # Fetch the release data
    return [release_info(user, project, tag) for tag in tags[:limit]]

if __name__ == "__main__":
    ## Parse arguments to locate the input files and options.
    parser = argparse.ArgumentParser(
        description='Generate releases information for an appstream file')

    parser.add_argument('--remote', metavar='REMOTE', type=str, action='store',
        help='Git remote to fetch release tags from')
    parser.add_argument('-v', '--verbose', default=False, action='store_true',
        help='Print verbose information about the translation file')
    args = parser.parse_args()

    # If no remote is provided, get it from the current working directory.
    if args.remote is None:
        args.remote = subprocess.check_output(
            ['git', 'remote', 'get-url', 'origin'],
            cwd=os.path.dirname(__file__),
        ).strip().decode('utf-8')

    # Fetch the release tags from
    releasedata = fetch_releases(args.remote)

    # Build the releases XML document.
    root = ElementTree.Element('releases')
    for release in releasedata:
        root.append(release.toxml())
    
    # Dump the XML data to stdout
    ElementTree.indent(root, space="  ")
    ElementTree.dump(root)
