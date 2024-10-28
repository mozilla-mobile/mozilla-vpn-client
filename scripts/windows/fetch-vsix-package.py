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
import requests
import shutil
import sys
import tempfile
import zipfile

def parse_vsman_packages(js):
    # Task a JSON blob and parse it into a searchable dict.
    return {pkg["id"]: pkg for pkg in js["packages"]}

def download_pkg_manifest(url):
    # Download the top-level manifest.
    top_req = requests.get(url)
    if top_req.status_code != 200:
        top_req.raise_for_status()

    # Parse it for the Visual studio package manifest.
    vsid = "Microsoft.VisualStudio.Manifests.VisualStudio"
    for item in top_req.json()["channelItems"]:
        if item["id"] != vsid:
            continue
        if len(item["payloads"]) != 1:
            continue

        vsman_req = requests.get(item["payloads"][0]["url"])
        if vsman_req.status_code != 200:
            vsman_req.raise_for_status()
        return parse_vsman_packages(vsman_req.json())

    raise LookupError(f"No manifest found for {vsid}")

def unpack_vsix(fileobj, output):
    # Load the VSIX file, it's really just a ZIP archive.
    with zipfile.ZipFile(fileobj) as zf:
        # Parse the manifest to see what to unpack.
        with zf.open("manifest.json", 'r') as fp:
            manifest = json.load(fp)
        
        # Unpack the desired files from the archive
        for x in manifest["files"]:
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
            with zf.open(filename, 'r') as infile:
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

    if name not in vsman:
        raise LookupError(f'No package found for {name}')

    # Print out what we're about to download.
    pkg = vsman[name]
    done.append(name)
    if "localizedResources" in pkg:
        print(f'Downloading: {pkg["localizedResources"][0]["title"]}')
    else:
        print(f'Downloading: {pkg["id"]}')

    # Download and extract the payloads.
    if "payloads" in pkg:
        for file in pkg["payloads"]:
            # Download the VSIX file
            with tempfile.TemporaryFile(suffix="-vsix.zip") as archive:
                r = requests.get(file["url"], stream=True)
                if r.status_code != 200:
                    r.raise_for_status()
                shutil.copyfileobj(r.raw, archive)
                archive.seek(0)

                unpack_vsix(archive, output)

    # If there are dependencies, fetch them recursively.
    if "dependencies" in pkg:
        for dep in pkg["dependencies"].keys():
            download_and_extract(vsman, dep, output, done)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Fetch and unpack Visual Studio Extensions')
    parser.add_argument('package', metavar='PKG', type=str, nargs='*',
                        help='VSIX Extension to fetch')
    parser.add_argument('-o', '--output', metavar='DIR', type=str, action='store', default=os.getcwd(),
                        help='Extract VSIX extensions to DIR')
    parser.add_argument('-r', '--manifest-version', metavar='VER', type=int, action='store', default=17,
                        help='Fetch manifest for Visual Studio version VER')
    args = parser.parse_args()

    # Fetch the top-level manifest and locate the Visual studio packages
    vsman = download_pkg_manifest(f"https://aka.ms/vs/{args.manifest_version}/release/channel")
    done = []
    for name in args.package:
        download_and_extract(vsman, name, args.output, done)
