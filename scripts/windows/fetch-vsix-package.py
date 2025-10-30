#! /usr/bin/env python3
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file is largely inspired by the Jake-Shadle/xwin project, but this
# allows us to download additional Visual Studio components that aren't
# covered by the xwin tool.

import argparse
import hashlib
import json
import os
import urllib.request
import shutil
import sys
import tempfile
import zipfile

def parse_vsman_packages(js):
    # Task a JSON blob and parse it into a searchable dict.
    return {pkg["id"]: pkg for pkg in js["packages"]}

def download_pkg_manifest(url):
    # Download the top-level manifest.
    with urllib.request.urlopen(url) as channels:
        channelItems = json.loads(channels.read())["channelItems"]

        # Parse it for the Visual studio package manifest.
        vsid = "Microsoft.VisualStudio.Manifests.VisualStudio"
        for item in channelItems:
            if item["id"] != vsid:
                continue
            if len(item["payloads"]) != 1:
                continue

            with urllib.request.urlopen(item["payloads"][0]["url"]) as vsman:
                return parse_vsman_packages(json.loads(vsman.read()))

        raise LookupError(f"No manifest found for {vsid}")

def unpack_vsix(fileobj, output):
    # Load the VSIX file, it's really just a ZIP archive.
    with zipfile.ZipFile(fileobj) as zf:
        # Parse the manifest to see what to unpack.
        with zf.open("manifest.json", 'r') as fp:
            manifest = json.load(fp)

        # Unpack the desired files from the archive, if any.
        for x in manifest.get("files", {}):
            # Select only the filenames in the Contents directory.
            filename = x["fileName"].lstrip('/')
            prefix, path = filename.split('/', maxsplit=1)
            if prefix != 'Contents':
                continue

            # The destination should be stripped of the Contents dir.
            dst = os.path.join(output, path)
            os.makedirs(os.path.split(dst)[0], exist_ok=True)

            # Extract the file and generate its hash at the same time.
            h = hashlib.sha256()
            with zf.open(filename.replace(' ', '%20'), 'r') as infile:
                with open(dst, 'wb') as outfile:
                    while True:
                        data = infile.read(65536)
                        if len(data) == 0:
                            break
                        h.update(data)
                        outfile.write(data)

            # Validate the checksum
            sha256 = x["sha256"].lower()
            if h.hexdigest().lower() != sha256:
                raise Exception(f'Invalid sha256 for {filename}')

def download_and_extract(vsman, name, output, done):
    # Skip dependencies we have already downloaded.
    if name in done:
        return
    elif name not in vsman:
        raise LookupError(f'No package found for {name}')
    else:
        done.append(name)

    # Print out what we're about to download.
    pkg = vsman[name]
    pkgtype = pkg['type']
    pretty = pkg["id"]
    if "localizedResources" in pkg:
        pretty = pkg["localizedResources"][0]["title"]

    # Download and extract the payloads.
    if pkgtype in ("Vsix"):
        print(f'Downloading: {pretty}')
        for file in pkg.get("payloads", []):
            # Download the VSIX file to disk so that it can be seekable.
            with tempfile.TemporaryFile(suffix="-vsix.zip") as archive:
                with urllib.request.urlopen(file["url"]) as r:
                    shutil.copyfileobj(r, archive)
                    archive.seek(0)
                    unpack_vsix(archive, output)
    elif pkgtype in ("Component", "Group"):
        print(f'Downloading {pkgtype}: {pretty}')
    else:
        print(f'Skipping {pkgtype}: {pretty}')
        return

    # If there are dependencies, fetch them recursively.
    if "dependencies" in pkg:
        for dep in pkg["dependencies"].keys():
            v = pkg["dependencies"][dep]
            if isinstance(v, dict):
                download_and_extract(vsman, v.get('id', dep), output, done)
            else:
                download_and_extract(vsman, dep, output, done)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Fetch and unpack Visual Studio Extensions')
    parser.add_argument('package', metavar='PKG', type=str, nargs='*',
                        help='VSIX Extension to fetch')
    parser.add_argument('-o', '--output', metavar='DIR', type=str, action='store', default=os.getcwd(),
                        help='Extract VSIX extensions to DIR')
    parser.add_argument('-r', '--manifest-version', metavar='VER', type=int, action='store', default=17,
                        help='Fetch manifest for Visual Studio version VER')
    parser.add_argument('-l', '--ltsc-version', metavar='VER', type=str, action='store',
                        help='Fetech LTSC manifest for Visual Studio version VER')
    args = parser.parse_args()

    # Fetch the top-level manifest and locate the Visual studio packages
    if args.ltsc_version is not None:
        major = args.ltsc_version.split('.')[0]
        vsman = download_pkg_manifest(f"https://aka.ms/vs/{major}/release.LTSC.{args.ltsc_version}/channel")
    else:
        vsman = download_pkg_manifest(f"https://aka.ms/vs/{args.manifest_version}/release/channel")
    done = []
    for name in args.package:
        download_and_extract(vsman, name, args.output, done)
